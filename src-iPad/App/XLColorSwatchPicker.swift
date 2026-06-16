import SwiftUI

// XLColorSwatchPicker — augments the system ColorPicker with the same
// affordances the desktop's xlColourPickerDialog provides:
//   (a) 140-entry CSS named-color swatch grid (20 cols × 7 rows)
//   (b) Persisted recent-colors strip (most-recent-first, deduped, cap 24)
//   (c) Hex entry field
//   (d) Embedded system ColorPicker for the freeform case
//
// Present via .popover or .sheet from a long-press / button tap.
// Call the `onCommit` closure to deliver the final hex string; the
// caller is responsible for recording the color into the recents store
// via `XLRecentColors.push(_:)`.

// MARK: - Persisted recent colors

struct XLRecentColors {
    private static let key = "xlights.recentColors"
    private static let cap = 24

    static func load() -> [String] {
        UserDefaults.standard.stringArray(forKey: key) ?? []
    }

    static func push(_ hex: String) {
        var list = load()
        let upper = hex.uppercased()
        list.removeAll { $0.uppercased() == upper }
        list.insert(upper, at: 0)
        if list.count > cap { list = Array(list.prefix(cap)) }
        UserDefaults.standard.set(list, forKey: key)
    }
}

// MARK: - CSS named colors (matches xlColourPickerDialog.cpp CSS_COLORS[])

private struct CSSColor {
    let name: String
    let r: UInt8
    let g: UInt8
    let b: UInt8
    var hex: String { String(format: "#%02X%02X%02X", r, g, b) }
    var color: Color { Color(.sRGB, red: Double(r)/255, green: Double(g)/255, blue: Double(b)/255, opacity: 1) }
}

private let cssColors: [CSSColor] = [
    CSSColor(name: "AliceBlue",             r: 240, g: 248, b: 255),
    CSSColor(name: "AntiqueWhite",          r: 250, g: 235, b: 215),
    CSSColor(name: "Aqua",                  r:   0, g: 255, b: 255),
    CSSColor(name: "Aquamarine",            r: 127, g: 255, b: 212),
    CSSColor(name: "Azure",                 r: 240, g: 255, b: 255),
    CSSColor(name: "Beige",                 r: 245, g: 245, b: 220),
    CSSColor(name: "Bisque",                r: 255, g: 228, b: 196),
    CSSColor(name: "Black",                 r:   0, g:   0, b:   0),
    CSSColor(name: "BlanchedAlmond",        r: 255, g: 235, b: 205),
    CSSColor(name: "Blue",                  r:   0, g:   0, b: 255),
    CSSColor(name: "BlueViolet",            r: 138, g:  43, b: 226),
    CSSColor(name: "Brown",                 r: 165, g:  42, b:  42),
    CSSColor(name: "BurlyWood",             r: 222, g: 184, b: 135),
    CSSColor(name: "CadetBlue",             r:  95, g: 158, b: 160),
    CSSColor(name: "Chartreuse",            r: 127, g: 255, b:   0),
    CSSColor(name: "Chocolate",             r: 210, g: 105, b:  30),
    CSSColor(name: "Coral",                 r: 255, g: 127, b:  80),
    CSSColor(name: "CornflowerBlue",        r: 100, g: 149, b: 237),
    CSSColor(name: "Cornsilk",              r: 255, g: 248, b: 220),
    CSSColor(name: "Crimson",               r: 220, g:  20, b:  60),
    CSSColor(name: "Cyan",                  r:   0, g: 255, b: 255),
    CSSColor(name: "DarkBlue",              r:   0, g:   0, b: 139),
    CSSColor(name: "DarkCyan",              r:   0, g: 139, b: 139),
    CSSColor(name: "DarkGoldenRod",         r: 184, g: 134, b:  11),
    CSSColor(name: "DarkGray",              r: 169, g: 169, b: 169),
    CSSColor(name: "DarkGreen",             r:   0, g: 100, b:   0),
    CSSColor(name: "DarkKhaki",             r: 189, g: 183, b: 107),
    CSSColor(name: "DarkMagenta",           r: 139, g:   0, b: 139),
    CSSColor(name: "DarkOliveGreen",        r:  85, g: 107, b:  47),
    CSSColor(name: "DarkOrange",            r: 255, g: 140, b:   0),
    CSSColor(name: "DarkOrchid",            r: 153, g:  50, b: 204),
    CSSColor(name: "DarkRed",               r: 139, g:   0, b:   0),
    CSSColor(name: "DarkSalmon",            r: 233, g: 150, b: 122),
    CSSColor(name: "DarkSeaGreen",          r: 143, g: 188, b: 143),
    CSSColor(name: "DarkSlateBlue",         r:  72, g:  61, b: 139),
    CSSColor(name: "DarkSlateGray",         r:  47, g:  79, b:  79),
    CSSColor(name: "DarkTurquoise",         r:   0, g: 206, b: 209),
    CSSColor(name: "DarkViolet",            r: 148, g:   0, b: 211),
    CSSColor(name: "DeepPink",              r: 255, g:  20, b: 147),
    CSSColor(name: "DeepSkyBlue",           r:   0, g: 191, b: 255),
    CSSColor(name: "DimGray",               r: 105, g: 105, b: 105),
    CSSColor(name: "DodgerBlue",            r:  30, g: 144, b: 255),
    CSSColor(name: "FireBrick",             r: 178, g:  34, b:  34),
    CSSColor(name: "FloralWhite",           r: 255, g: 250, b: 240),
    CSSColor(name: "ForestGreen",           r:  34, g: 139, b:  34),
    CSSColor(name: "Fuchsia",               r: 255, g:   0, b: 255),
    CSSColor(name: "Gainsboro",             r: 220, g: 220, b: 220),
    CSSColor(name: "GhostWhite",            r: 248, g: 248, b: 255),
    CSSColor(name: "Gold",                  r: 255, g: 215, b:   0),
    CSSColor(name: "GoldenRod",             r: 218, g: 165, b:  32),
    CSSColor(name: "Gray",                  r: 128, g: 128, b: 128),
    CSSColor(name: "Green",                 r:   0, g: 128, b:   0),
    CSSColor(name: "GreenYellow",           r: 173, g: 255, b:  47),
    CSSColor(name: "HoneyDew",              r: 240, g: 255, b: 240),
    CSSColor(name: "HotPink",               r: 255, g: 105, b: 180),
    CSSColor(name: "IndianRed",             r: 205, g:  92, b:  92),
    CSSColor(name: "Indigo",                r:  75, g:   0, b: 130),
    CSSColor(name: "Ivory",                 r: 255, g: 255, b: 240),
    CSSColor(name: "Khaki",                 r: 240, g: 230, b: 140),
    CSSColor(name: "Lavender",              r: 230, g: 230, b: 250),
    CSSColor(name: "LavenderBlush",         r: 255, g: 240, b: 245),
    CSSColor(name: "LawnGreen",             r: 124, g: 252, b:   0),
    CSSColor(name: "LemonChiffon",          r: 255, g: 250, b: 205),
    CSSColor(name: "LightBlue",             r: 173, g: 216, b: 230),
    CSSColor(name: "LightCoral",            r: 240, g: 128, b: 128),
    CSSColor(name: "LightCyan",             r: 224, g: 255, b: 255),
    CSSColor(name: "LightGoldenRodYellow",  r: 250, g: 250, b: 210),
    CSSColor(name: "LightGray",             r: 211, g: 211, b: 211),
    CSSColor(name: "LightGreen",            r: 144, g: 238, b: 144),
    CSSColor(name: "LightPink",             r: 255, g: 182, b: 193),
    CSSColor(name: "LightSalmon",           r: 255, g: 160, b: 122),
    CSSColor(name: "LightSeaGreen",         r:  32, g: 178, b: 170),
    CSSColor(name: "LightSkyBlue",          r: 135, g: 206, b: 250),
    CSSColor(name: "LightSlateGray",        r: 119, g: 136, b: 153),
    CSSColor(name: "LightSteelBlue",        r: 176, g: 196, b: 222),
    CSSColor(name: "LightYellow",           r: 255, g: 255, b: 224),
    CSSColor(name: "Lime",                  r:   0, g: 255, b:   0),
    CSSColor(name: "LimeGreen",             r:  50, g: 205, b:  50),
    CSSColor(name: "Linen",                 r: 250, g: 240, b: 230),
    CSSColor(name: "Magenta",               r: 255, g:   0, b: 255),
    CSSColor(name: "Maroon",                r: 128, g:   0, b:   0),
    CSSColor(name: "MediumAquaMarine",      r: 102, g: 205, b: 170),
    CSSColor(name: "MediumBlue",            r:   0, g:   0, b: 205),
    CSSColor(name: "MediumOrchid",          r: 186, g:  85, b: 211),
    CSSColor(name: "MediumPurple",          r: 147, g: 112, b: 219),
    CSSColor(name: "MediumSeaGreen",        r:  60, g: 179, b: 113),
    CSSColor(name: "MediumSlateBlue",       r: 123, g: 104, b: 238),
    CSSColor(name: "MediumSpringGreen",     r:   0, g: 250, b: 154),
    CSSColor(name: "MediumTurquoise",       r:  72, g: 209, b: 204),
    CSSColor(name: "MediumVioletRed",       r: 199, g:  21, b: 133),
    CSSColor(name: "MidnightBlue",          r:  25, g:  25, b: 112),
    CSSColor(name: "MintCream",             r: 245, g: 255, b: 250),
    CSSColor(name: "MistyRose",             r: 255, g: 228, b: 225),
    CSSColor(name: "Moccasin",              r: 255, g: 228, b: 181),
    CSSColor(name: "NavajoWhite",           r: 255, g: 222, b: 173),
    CSSColor(name: "Navy",                  r:   0, g:   0, b: 128),
    CSSColor(name: "OldLace",               r: 253, g: 245, b: 230),
    CSSColor(name: "Olive",                 r: 128, g: 128, b:   0),
    CSSColor(name: "OliveDrab",             r: 107, g: 142, b:  35),
    CSSColor(name: "Orange",                r: 255, g: 165, b:   0),
    CSSColor(name: "OrangeRed",             r: 255, g:  69, b:   0),
    CSSColor(name: "Orchid",                r: 218, g: 112, b: 214),
    CSSColor(name: "PaleGoldenRod",         r: 238, g: 232, b: 170),
    CSSColor(name: "PaleGreen",             r: 152, g: 251, b: 152),
    CSSColor(name: "PaleTurquoise",         r: 175, g: 238, b: 238),
    CSSColor(name: "PaleVioletRed",         r: 219, g: 112, b: 147),
    CSSColor(name: "PapayaWhip",            r: 255, g: 239, b: 213),
    CSSColor(name: "PeachPuff",             r: 255, g: 218, b: 185),
    CSSColor(name: "Peru",                  r: 205, g: 133, b:  63),
    CSSColor(name: "Pink",                  r: 255, g: 192, b: 203),
    CSSColor(name: "Plum",                  r: 221, g: 160, b: 221),
    CSSColor(name: "PowderBlue",            r: 176, g: 224, b: 230),
    CSSColor(name: "Purple",                r: 128, g:   0, b: 128),
    CSSColor(name: "Red",                   r: 255, g:   0, b:   0),
    CSSColor(name: "RosyBrown",             r: 188, g: 143, b: 143),
    CSSColor(name: "RoyalBlue",             r:  65, g: 105, b: 225),
    CSSColor(name: "SaddleBrown",           r: 139, g:  69, b:  19),
    CSSColor(name: "Salmon",                r: 250, g: 128, b: 114),
    CSSColor(name: "SandyBrown",            r: 244, g: 164, b:  96),
    CSSColor(name: "SeaGreen",              r:  46, g: 139, b:  87),
    CSSColor(name: "SeaShell",              r: 255, g: 245, b: 238),
    CSSColor(name: "Sienna",                r: 160, g:  82, b:  45),
    CSSColor(name: "Silver",                r: 192, g: 192, b: 192),
    CSSColor(name: "SkyBlue",               r: 135, g: 206, b: 235),
    CSSColor(name: "SlateBlue",             r: 106, g:  90, b: 205),
    CSSColor(name: "SlateGray",             r: 112, g: 128, b: 144),
    CSSColor(name: "Snow",                  r: 255, g: 250, b: 250),
    CSSColor(name: "SpringGreen",           r:   0, g: 255, b: 127),
    CSSColor(name: "SteelBlue",             r:  70, g: 130, b: 180),
    CSSColor(name: "Tan",                   r: 210, g: 180, b: 140),
    CSSColor(name: "Teal",                  r:   0, g: 128, b: 128),
    CSSColor(name: "Thistle",               r: 216, g: 191, b: 216),
    CSSColor(name: "Tomato",                r: 255, g:  99, b:  71),
    CSSColor(name: "Turquoise",             r:  64, g: 224, b: 208),
    CSSColor(name: "Violet",                r: 238, g: 130, b: 238),
    CSSColor(name: "Wheat",                 r: 245, g: 222, b: 179),
    CSSColor(name: "White",                 r: 255, g: 255, b: 255),
    CSSColor(name: "WhiteSmoke",            r: 245, g: 245, b: 245),
    CSSColor(name: "Yellow",                r: 255, g: 255, b:   0),
    CSSColor(name: "YellowGreen",           r: 154, g: 205, b:  50),
]

// MARK: - Picker sheet

struct XLColorSwatchPicker: View {
    let initialHex: String
    let onCommit: (String) -> Void

    @State private var hexField: String = ""
    @State private var pickerColor: Color = .black
    @State private var recents: [String] = []
    @State private var hexError = false

    private let cols = 20
    private let swatchSize: CGFloat = 22

    var body: some View {
        NavigationStack {
            ScrollView {
                VStack(alignment: .leading, spacing: 10) {
                    recentSection
                    cssSection
                    freeformSection
                }
                .padding()
            }
            .navigationTitle("Pick Color")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { commitHex(hexField) }
                }
            }
        }
        .onAppear {
            hexField = initialHex.uppercased().hasPrefix("#")
                ? String(initialHex.uppercased())
                : "#\(initialHex.uppercased())"
            pickerColor = colorFromHex(hexField) ?? .black
            recents = XLRecentColors.load()
        }
    }

    // MARK: Recent strip

    @ViewBuilder
    private var recentSection: some View {
        if !recents.isEmpty {
            VStack(alignment: .leading, spacing: 4) {
                Text("Recent Colors")
                    .font(.caption)
                    .fontWeight(.medium)
                    .foregroundStyle(.secondary)
                LazyVGrid(columns: Array(repeating: GridItem(.fixed(swatchSize), spacing: 2), count: cols),
                          spacing: 2) {
                    ForEach(recents.prefix(cols), id: \.self) { hex in
                        swatchCell(hex: hex, label: hex)
                    }
                }
            }
        }
    }

    // MARK: CSS named-color grid

    private var cssSection: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text("Named Colors")
                .font(.caption)
                .fontWeight(.medium)
                .foregroundStyle(.secondary)
            LazyVGrid(columns: Array(repeating: GridItem(.fixed(swatchSize), spacing: 2), count: cols),
                      spacing: 2) {
                ForEach(cssColors.indices, id: \.self) { i in
                    let c = cssColors[i]
                    swatchCell(hex: c.hex, label: "\(c.name)  \(c.hex)")
                }
            }
        }
    }

    // MARK: Freeform section

    private var freeformSection: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Custom")
                .font(.caption)
                .fontWeight(.medium)
                .foregroundStyle(.secondary)
            HStack(spacing: 8) {
                TextField("#RRGGBB", text: $hexField)
                    .font(.system(.body, design: .monospaced))
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.characters)
                    .onChange(of: hexField) { _, v in
                        hexError = false
                        if let c = colorFromHex(v) { pickerColor = c }
                    }
                    .overlay(
                        RoundedRectangle(cornerRadius: 6)
                            .stroke(hexError ? Color.red : Color.clear, lineWidth: 1.5)
                    )
                    .frame(maxWidth: 120)
                    .textFieldStyle(.roundedBorder)
                Button("Set") { commitHex(hexField) }
                    .buttonStyle(.bordered)
                ColorPicker("", selection: Binding(
                    get: { pickerColor },
                    set: { c in
                        pickerColor = c
                        if let hex = hexFromColor(c) {
                            hexField = hex
                            hexError = false
                        }
                    }), supportsOpacity: false)
                    .labelsHidden()
                    .frame(width: 44)
            }
            if let c = colorFromHex(hexField) {
                RoundedRectangle(cornerRadius: 6)
                    .fill(c)
                    .frame(height: 28)
                    .overlay(
                        RoundedRectangle(cornerRadius: 6)
                            .stroke(Color.secondary.opacity(0.3), lineWidth: 0.5)
                    )
            }
        }
    }

    // MARK: Swatch cell

    private func swatchCell(hex: String, label: String) -> some View {
        Button {
            selectHex(hex)
        } label: {
            RoundedRectangle(cornerRadius: 2)
                .fill(colorFromHex(hex) ?? .gray)
                .frame(width: swatchSize, height: swatchSize)
                .overlay(
                    RoundedRectangle(cornerRadius: 2)
                        .stroke(Color.primary.opacity(0.2), lineWidth: 0.5)
                )
        }
        .buttonStyle(.plain)
        .accessibilityLabel(label)
    }

    // MARK: Helpers

    private func selectHex(_ hex: String) {
        let upper = hex.uppercased().hasPrefix("#") ? hex.uppercased() : "#\(hex.uppercased())"
        hexField = upper
        if let c = colorFromHex(upper) { pickerColor = c }
        hexError = false
        commitHex(upper)
    }

    private func commitHex(_ hex: String) {
        guard colorFromHex(hex) != nil else {
            hexError = true
            return
        }
        let upper = hex.uppercased().hasPrefix("#") ? hex.uppercased() : "#\(hex.uppercased())"
        XLRecentColors.push(upper)
        recents = XLRecentColors.load()
        onCommit(upper)
    }
}
