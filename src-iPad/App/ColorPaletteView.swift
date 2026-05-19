import SwiftUI

// 8-slot color palette picker used by the Color shared panel. Each slot has:
//   - An enable toggle (C_CHECKBOX_Palette1..8)
//   - A color picker OR a ColorCurve gradient editor
//     (C_BUTTON_Palette1..8, stored as either "#RRGGBB" hex or a
//     ColorCurve `Active=TRUE|…` serialised blob)
// Disabled slots are ignored by the renderer but their color is preserved.
//
// Slots containing a ColorCurve render the gradient inline + a mode
// badge (↔ for linear, ◎ for radial, ↻ for rotation). Tap the
// gradient strip or long-press any slot → ColorCurveEditorSheet.
struct ColorPaletteView: View {
    @Environment(SequencerViewModel.self) var viewModel

    @State private var editingSlot: Int? = nil
    @State private var showingLoadSheet = false
    @State private var showingImportSheet = false
    @State private var showingSaveAsSheet = false
    @State private var showingAISheet = false

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            HStack(spacing: 6) {
                Text("Palette")
                    .font(.caption)
                    .fontWeight(.medium)
                    .foregroundStyle(.secondary)
                Spacer()
                Menu {
                    paletteMenuContent()
                } label: {
                    Image(systemName: "ellipsis.circle")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                .menuStyle(.borderlessButton)
            }

            VStack(spacing: 4) {
                ForEach(1...8, id: \.self) { slot in
                    paletteRow(slot: slot)
                }
            }
        }
        .padding(.vertical, 4)
        .sheet(item: Binding(
            get: { editingSlot.map { SlotRef(id: $0) } },
            set: { editingSlot = $0?.id }
        )) { ref in
            let support = colorCurveSupport()
            ColorCurveEditorSheet(
                slot: ref.id,
                storedString: viewModel.settingValue(
                    forKey: "C_BUTTON_Palette\(ref.id)",
                    defaultValue: ""),
                supportsLinear: support.linear,
                supportsRadial: support.radial
            )
            .environment(viewModel)
        }
        .sheet(isPresented: $showingLoadSheet) {
            PaletteLoadSheet(onApply: applyPalette)
                .environment(viewModel)
        }
        .sheet(isPresented: $showingImportSheet) {
            PaletteImportSheet(onImport: applyPalette)
        }
        .sheet(isPresented: $showingSaveAsSheet) {
            PaletteSaveAsSheet { name in
                _ = viewModel.document.savePaletteString(
                    currentPaletteString(), asName: name)
            }
        }
        .sheet(isPresented: $showingAISheet) {
            AIPaletteGenerationSheet()
                .environment(viewModel)
        }
    }

    // MARK: - Palette menu (save / load / import / export)

    @ViewBuilder
    private func paletteMenuContent() -> some View {
        Button {
            _ = viewModel.document.savePaletteString(
                currentPaletteString(), asName: nil)
        } label: {
            Label("Save Palette", systemImage: "square.and.arrow.down")
        }
        Button {
            showingSaveAsSheet = true
        } label: {
            Label("Save Palette As…", systemImage: "square.and.arrow.down.on.square")
        }
        Button {
            showingLoadSheet = true
        } label: {
            Label("Load Saved Palette…", systemImage: "folder")
        }
        Divider()
        if XLAIServices.shared().hasEnabledService(forCapability: XLAICapabilityColorPalettes) {
            Button {
                showingAISheet = true
            } label: {
                Label("AI Generate Palette…", systemImage: "wand.and.stars")
            }
        }
        Button {
            showingImportSheet = true
        } label: {
            Label("Import from Text…", systemImage: "text.badge.plus")
        }
        Button {
            UIPasteboard.general.string = currentPaletteString()
        } label: {
            Label("Copy Palette String", systemImage: "doc.on.doc")
        }
    }

    private func currentPaletteString() -> String {
        guard let sel = viewModel.selectedEffect else { return "" }
        return viewModel.document.currentPaletteString(
            forRow: Int32(sel.rowIndex),
            at: Int32(sel.effectIndex))
    }

    private func applyPalette(_ paletteString: String) {
        guard let sel = viewModel.selectedEffect else { return }
        _ = viewModel.document.applyPaletteString(
            paletteString,
            toRow: Int32(sel.rowIndex),
            at: Int32(sel.effectIndex))
        // Kick the view-model's settings cache so SwiftUI redraws.
        viewModel.refreshSelectedEffectSettings()
    }

    private struct SlotRef: Identifiable { let id: Int }

    /// Probe the currently-selected effect's ColorCurve mode support.
    /// If nothing is selected, assume both are supported (editor
    /// behaves permissively rather than locking out controls for
    /// no reason).
    private func colorCurveSupport() -> (linear: Bool, radial: Bool) {
        guard let sel = viewModel.selectedEffect else { return (true, true) }
        let dict = viewModel.document.colorCurveModeSupport(
            forRow: Int32(sel.rowIndex),
            at: Int32(sel.effectIndex))
        let linear = dict["linear"]?.boolValue ?? true
        let radial = dict["radial"]?.boolValue ?? true
        return (linear, radial)
    }

    private func paletteRow(slot: Int) -> some View {
        let checkboxKey = "C_CHECKBOX_Palette\(slot)"
        let buttonKey = "C_BUTTON_Palette\(slot)"

        // Default palette colors — match desktop's first-run palette.
        let defaultHex = ["#FF0000", "#00FF00", "#0000FF", "#FFFF00",
                          "#FFFFFF", "#000000", "#FFA500", "#800080"][slot - 1]

        let enabledBinding = Binding<Bool>(
            get: {
                // Default to off for every slot, including slot 1. A
                // previous heuristic defaulted slot 1 to "1" when the
                // map had no entry, which made Palette 1 look forced-on
                // for any effect whose saved palette genuinely has
                // slot 1 disabled. `Effect::ParseColorMap` treats a
                // missing checkbox key as false, so matching that here
                // is what keeps the render and the UI in sync.
                let v = viewModel.settingValue(forKey: checkboxKey, defaultValue: "0")
                return v == "1" || v.lowercased() == "true"
            },
            set: { viewModel.setSettingValue($0 ? "1" : "0", forKey: checkboxKey) }
        )

        let rawValue = viewModel.settingValue(forKey: buttonKey, defaultValue: defaultHex)
        let isColorCurve = rawValue.hasPrefix("Active=TRUE")

        let colorBinding = Binding<Color>(
            get: {
                if isColorCurve { return .gray }
                return colorFromHex(rawValue) ?? .black
            },
            set: { newColor in
                if let hex = hexFromColor(newColor) {
                    viewModel.setSettingValue(hex, forKey: buttonKey)
                }
            }
        )

        return HStack(spacing: 8) {
            Toggle("", isOn: enabledBinding)
                .labelsHidden()
                .toggleStyle(.switch)
                .controlSize(.small)

            Text("\(slot)")
                .font(.caption2)
                .foregroundStyle(.secondary)
                .frame(width: 12)

            if isColorCurve {
                // Render the actual gradient so the user can tell
                // gradients apart at a glance instead of every
                // curve-slot looking like every other curve-slot.
                // A mode badge in the top-right shows whether it's
                // time / linear / radial / rotational so they don't
                // have to open the editor to check.
                CurveSlotPreview(serialised: rawValue,
                                 identifier: "Palette\(slot)")
                    .frame(height: 28)
                    .onTapGesture { editingSlot = slot }
            } else {
                ColorPicker("", selection: colorBinding, supportsOpacity: false)
                    .labelsHidden()
                    .frame(maxWidth: .infinity, alignment: .leading)
            }
        }
        // Long-press any slot → open editor. For plain-hex slots this
        // is how the user converts the slot into a curve (editor
        // toggle opens in "Use gradient = off" state; flipping it on
        // seeds a black→white ramp).
        .contextMenu {
            Button {
                editingSlot = slot
            } label: {
                Label(isColorCurve ? "Edit Gradient…" : "Edit as Gradient…",
                      systemImage: "paintpalette")
            }
            if isColorCurve {
                Button(role: .destructive) {
                    // Strip the curve back to a plain colour.
                    viewModel.setSettingValue(defaultHex, forKey: buttonKey)
                } label: {
                    Label("Convert to Plain Colour", systemImage: "circle.fill")
                }
            }
        }
    }

    private func colorFromHex(_ hex: String) -> Color? {
        var s = hex.trimmingCharacters(in: .whitespaces)
        if s.hasPrefix("#") { s.removeFirst() }
        guard s.count == 6 || s.count == 8, let val = UInt64(s, radix: 16) else { return nil }
        let r = Double((val >> 16) & 0xFF) / 255.0
        let g = Double((val >> 8) & 0xFF) / 255.0
        let b = Double(val & 0xFF) / 255.0
        // sRGB-pinned so exact #RRGGBB hex the user types
        // round-trips identically — see ColorPanelCustomRows.swift's
        // top-level colorFromHex for the full rationale.
        return Color(.sRGB, red: r, green: g, blue: b, opacity: 1)
    }

    // Inline gradient thumbnail for palette slots holding a
    // ColorCurve. Samples the curve along its x axis via
    // `XLColorCurve.colorAt(offset:)` and overlays a mode-hint
    // glyph (↔, ↕, ◎, ↻) in the top-right.
    struct CurveSlotPreview: View {
        let serialised: String
        let identifier: String

        var body: some View {
            let core = XLColorCurve(serialised: serialised,
                                    identifier: identifier)
            RoundedRectangle(cornerRadius: 4)
                .fill(Color.clear)
                .overlay(
                    Canvas { ctx, size in
                        let w = Int(size.width)
                        guard w > 0 else { return }
                        for px in 0..<w {
                            let frac = Float(px) / Float(w)
                            let c = Color(uiColor: core.color(atOffset: frac))
                            let r = CGRect(x: CGFloat(px), y: 0,
                                           width: 1, height: size.height)
                            ctx.fill(Path(r), with: .color(c))
                        }
                    }
                )
                .clipShape(RoundedRectangle(cornerRadius: 4))
                .overlay(alignment: .topTrailing) {
                    if let glyph = badgeGlyph(for: core.mode) {
                        Text(glyph)
                            .font(.caption2.bold())
                            .foregroundStyle(.white)
                            .padding(.horizontal, 4)
                            .padding(.vertical, 1)
                            .background(
                                Capsule()
                                    .fill(Color.black.opacity(0.55))
                            )
                            .padding(3)
                    }
                }
                .overlay(
                    RoundedRectangle(cornerRadius: 4)
                        .stroke(Color.secondary.opacity(0.3), lineWidth: 0.5)
                )
        }

        private func badgeGlyph(for mode: XLColorCurveMode) -> String? {
            switch mode {
            case .time:       return nil      // default — no badge
            case .right, .left: return "↔"
            case .up, .down:  return "↕"
            case .radialIn, .radialOut: return "◎"
            case .cw, .ccw:   return "↻"
            @unknown default: return nil
            }
        }
    }

    private func hexFromColor(_ color: Color) -> String? {
        #if canImport(UIKit)
        // On wide-gamut iPads SwiftUI Color may be in Display P3; convert to
        // sRGB so out-of-gamut components don't escape [0,1] and produce bad hex.
        guard let srgb = CGColorSpace(name: CGColorSpace.sRGB),
              let srgbCG = UIColor(color).cgColor.converted(to: srgb, intent: .defaultIntent, options: nil),
              let c = srgbCG.components, c.count >= 3 else { return nil }
        let ri = Int((max(0, min(1, c[0])) * 255).rounded())
        let gi = Int((max(0, min(1, c[1])) * 255).rounded())
        let bi = Int((max(0, min(1, c[2])) * 255).rounded())
        return String(format: "#%02X%02X%02X", ri, gi, bi)
        #else
        return nil
        #endif
    }
}
