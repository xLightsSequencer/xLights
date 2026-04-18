import Foundation

// Swift Codable models mirroring resources/effectmetadata/_schema.json
//
// See also:
//   resources/effectmetadata/<EffectName>.json — per-effect metadata
//   resources/effectmetadata/shared/Buffer.json   — buffer panel (B_ prefix)
//   resources/effectmetadata/shared/Color.json    — color panel (C_ prefix)
//   resources/effectmetadata/shared/Blending.json — layer-blending panel
//                                                   (T_ prefix; file was
//                                                   previously Timing.json)
//
// The desktop wx UI consumes these via src-ui-wx/ui/effectpanels/JsonEffectPanel.cpp;
// this Swift layer re-implements the same controls for the iPad.

struct EffectMetadata: Codable {
    let effectName: String
    let prefix: String?                 // "B_", "C_", "T_" for shared; nil/empty → "E_"
    let canvasMode: Bool?
    let scrollable: Bool?
    let properties: [PropertyMetadata]?
    let groups: [GroupMetadata]?
    let visibilityRules: [VisibilityRuleMetadata]?
    let notes: [String]?

    /// Prefix applied when computing setting keys. Effects default to "E_".
    var settingKeyPrefix: String {
        guard let p = prefix, !p.isEmpty else { return "E_" }
        return p
    }
}

struct PropertyMetadata: Codable {
    let id: String
    let label: String
    let tooltip: String?
    let type: String                    // "int", "float", "bool", "enum", "string", "color", "file", "font"
    let controlType: String             // "slider", "checkbox", "choice", "spin", "text", "custom", ...
    let defaultValue: AnyCodable?
    let min: Double?
    let max: Double?
    let divisor: Int?
    let valueCurve: Bool?
    let vcMin: Double?
    let vcMax: Double?
    let lockable: Bool?
    let options: [String]?
    let dynamicOptions: String?
    let separator: Bool?
    let suppressIfDefault: Bool?
    let fileFilter: String?
    let fileMessage: String?
    let settingPrefix: String?          // "TEXTCTRL" or "SLIDER" override for floats
    let fullWidth: Bool?
    let expandToFill: Bool?
    let growable: Bool?
    let checkboxLabel: String?
    let description: String?

    enum CodingKeys: String, CodingKey {
        case id, label, tooltip, type, controlType
        case defaultValue = "default"
        case min, max, divisor, valueCurve, vcMin, vcMax, lockable
        case options, dynamicOptions, separator, suppressIfDefault
        case fileFilter, fileMessage, settingPrefix
        case fullWidth, expandToFill, growable, checkboxLabel, description
    }

    /// Float values stored as int with divisor. Slider min/max are raw ints; the
    /// displayed value is value/divisor. A divisor of 1 (or nil) means int.
    var effectiveDivisor: Int { Swift.max(divisor ?? 1, 1) }

    /// The control-type prefix used to build the full setting key.
    /// Mirrors JsonEffectPanel.cpp:1699-1723.
    var controlTypePrefix: String {
        if let sp = settingPrefix, !sp.isEmpty {
            return sp
        }
        switch controlType {
        case "slider":
            // Float sliders: divisor > 1 → TEXTCTRL (formatted), unless settingPrefix forces SLIDER.
            if effectiveDivisor > 1 { return "TEXTCTRL" }
            return "SLIDER"
        case "checkbox", "togglebutton":    return "CHECKBOX"
        case "choice", "combobox":          return "CHOICE"
        case "spin":                        return "SPINCTRL"
        case "text":                        return "TEXTCTRL"
        case "filepicker":                  return "FILEPICKER"
        case "fontpicker":                  return "FONTPICKER"
        case "colourpicker":                return "COLOURPICKER"
        case "radiobutton":                 return "RADIOBUTTON"
        default:                            return "TEXTCTRL"
        }
    }

    /// Build the full setting key for this property given the owning metadata's prefix.
    func settingKey(prefix: String) -> String {
        return "\(prefix)\(controlTypePrefix)_\(id)"
    }

    /// Default value as string (what the underlying SettingsMap stores).
    func defaultAsString() -> String {
        guard let v = defaultValue else { return "" }
        return v.stringValue
    }
}

struct GroupMetadata: Codable {
    let type: String                    // "tabs", "section", "xyCenter"
    let label: String?
    let properties: [String]?           // for "section"
    let tabs: [TabDef]?                 // for "tabs"
    let xProperty: String?              // for "xyCenter"
    let yProperty: String?
    let wrapX: String?
    let wrapY: String?
    let settingKey: String?
    let persistSelection: Bool?
    let scrollable: Bool?

    struct TabDef: Codable {
        let label: String
        let properties: [String]
    }
}

struct VisibilityRuleMetadata: Codable {
    let when: WhenCondition
    let enable: [String]?
    let disable: [String]?
    let show: [String]?
    let hide: [String]?

    struct WhenCondition: Codable {
        let property: String?
        let equals: AnyCodable?
        let notEquals: AnyCodable?
        let oneOf: [AnyCodable]?
        let any: [String]?
    }
}

/// Type-erased Codable value. Used for fields whose JSON type can be string,
/// number, or bool depending on the property `type`.
struct AnyCodable: Codable {
    let value: Any

    init(from decoder: Decoder) throws {
        let c = try decoder.singleValueContainer()
        if let b = try? c.decode(Bool.self) {
            value = b
        } else if let i = try? c.decode(Int.self) {
            value = i
        } else if let d = try? c.decode(Double.self) {
            value = d
        } else if let s = try? c.decode(String.self) {
            value = s
        } else if c.decodeNil() {
            value = NSNull()
        } else {
            value = ""
        }
    }

    func encode(to encoder: Encoder) throws {
        var c = encoder.singleValueContainer()
        switch value {
        case let b as Bool:     try c.encode(b)
        case let i as Int:      try c.encode(i)
        case let d as Double:   try c.encode(d)
        case let s as String:   try c.encode(s)
        default:                try c.encodeNil()
        }
    }

    /// Canonical string representation matching how the desktop serializes values.
    var stringValue: String {
        switch value {
        case let b as Bool:     return b ? "1" : "0"
        case let i as Int:      return String(i)
        case let d as Double:
            // Match wx's behavior: integer doubles serialize without decimal.
            if d == d.rounded() { return String(Int(d)) }
            return String(d)
        case let s as String:   return s
        default:                return ""
        }
    }
}

// MARK: - Metadata Cache

/// Parses a raw JSON string from the bridge into an EffectMetadata struct.
/// Returns nil if parsing fails or the string is empty.
func parseEffectMetadata(_ jsonString: String) -> EffectMetadata? {
    guard !jsonString.isEmpty, let data = jsonString.data(using: .utf8) else { return nil }
    do {
        return try JSONDecoder().decode(EffectMetadata.self, from: data)
    } catch {
        print("parseEffectMetadata failed: \(error)")
        return nil
    }
}
