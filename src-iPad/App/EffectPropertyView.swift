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
                viewModel.setSettingValue(String(Int(newVal)), forKey: settingKey)
            }
        )

        return VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text(property.label)
                    .font(.caption)
                Spacer()
                Text(divisor > 1
                     ? String(format: "%.\(divisorDecimals(divisor))f", displayValue)
                     : "\(storedInt)")
                    .monospacedDigit()
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            Slider(value: binding, in: minVal...maxVal, step: 1)
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
                viewModel.setSettingValue(newVal ? "1" : "0", forKey: settingKey)
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
            set: { viewModel.setSettingValue($0, forKey: settingKey) }
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
            set: { viewModel.setSettingValue(String($0), forKey: settingKey) }
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
            set: { viewModel.setSettingValue($0, forKey: settingKey) }
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
