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
        case "custom":
            customView
        default:
            unsupportedPlaceholder
        }
    }

    // MARK: - Custom property dispatcher

    // Route known custom property ids to their bespoke implementations.
    // Unknown custom rows show a neutral placeholder.
    @ViewBuilder
    private var customView: some View {
        switch property.id {
        case "PaletteHeaderRow":
            ColorPaletteView()
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
        let displayValue = Double(storedInt) / Double(divisor)

        let binding = Binding<Double>(
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

        return VStack(alignment: .leading, spacing: 2) {
            HStack(spacing: 6) {
                Text(property.label)
                    .font(.caption)
                Spacer()
                Text(divisor > 1
                     ? String(format: "%.\(divisorDecimals(divisor))f", displayValue)
                     : "\(storedInt)")
                    .monospacedDigit()
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                if property.valueCurve == true {
                    ValueCurveButton(property: property, prefix: metadataPrefix)
                }
            }
            Slider(value: binding, in: minVal...maxVal, step: 1)
                .opacity(vcActive ? 0.4 : 1.0)
                .disabled(vcActive)
        }
        .padding(.vertical, 2)
    }

    private func divisorDecimals(_ divisor: Int) -> Int {
        switch divisor {
        case 10: return 1
        case 100: return 2
        case 1000: return 3
        default: return 2
        }
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
        }
        .toggleStyle(.switch)
        .padding(.vertical, 2)
    }

    // MARK: - Choice / Combobox

    private var choiceView: some View {
        let options = property.options ?? []
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
    }

    // MARK: - Spin (Stepper)

    private var spinView: some View {
        let minVal = Int(property.min ?? 0)
        let maxVal = Int(property.max ?? 100)
        let binding = Binding<Int>(
            get: { Int(rawValue) ?? Int(defaultValueString) ?? 0 },
            set: { writeValue(String($0)) }
        )
        return HStack {
            Text(property.label)
                .font(.caption)
            Spacer()
            Stepper(value: binding, in: minVal...maxVal) {
                Text("\(binding.wrappedValue)")
                    .monospacedDigit()
                    .font(.caption2)
                    .frame(minWidth: 36, alignment: .trailing)
            }
            .labelsHidden()
        }
        .padding(.vertical, 2)
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
    }

    // MARK: - Custom / Unsupported

    private var customPlaceholder: some View {
        HStack {
            Text(property.label.isEmpty ? property.id : property.label)
                .font(.caption)
                .foregroundStyle(.secondary)
            Spacer()
            Text("(custom)")
                .font(.caption2)
                .foregroundStyle(.tertiary)
        }
        .padding(.vertical, 2)
    }

    private var unsupportedPlaceholder: some View {
        HStack {
            Text(property.label.isEmpty ? property.id : property.label)
                .font(.caption)
                .foregroundStyle(.secondary)
            Spacer()
            Text("(\(property.controlType))")
                .font(.caption2)
                .foregroundStyle(.tertiary)
        }
        .padding(.vertical, 2)
    }
}
