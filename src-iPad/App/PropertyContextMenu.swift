import SwiftUI
import UIKit

// Long-press context menu for primitive property rows in the effect
// inspector — desktop parity for the per-control right-click actions
// (Gap G9) and the per-setting clipboard (Gap G13).
//
// Menu items:
//   - Copy Value      — pasteboard; prefixed so random text pastes don't
//                       get mistaken for property values
//   - Paste Value     — shown only if pasteboard has our prefix and the
//                       decoded value differs from the current one
//   - Reset to Default — writes the metadata default with suppressIfDefault
//                        so the key is removed (matches a fresh effect)
//   - Edit Value Curve — when property.valueCurve == true; opens the
//                        same sheet ValueCurveButton does
//
// Attach via `.propertyContextMenu(property:prefix:)` on any row that
// renders a single property's primitive control.
struct PropertyContextMenu: ViewModifier {
    @Environment(SequencerViewModel.self) var viewModel
    let property: PropertyMetadata
    let metadataPrefix: String

    @State private var showVCEditor = false

    private static let clipboardPrefix = "xlprop:v1:"

    private var settingKey: String { property.settingKey(prefix: metadataPrefix) }
    private var defaultValueString: String { property.defaultAsString() }
    private var rawValue: String {
        viewModel.settingValue(forKey: settingKey, defaultValue: defaultValueString)
    }
    private var isAtDefault: Bool { rawValue == defaultValueString }

    func body(content: Content) -> some View {
        content
            .contextMenu { menuContent() }
            .sheet(isPresented: $showVCEditor) {
                let vcKey = property.valueCurveKey(prefix: metadataPrefix)
                ValueCurveEditorSheet(property: property,
                                      prefix: metadataPrefix,
                                      storedString: viewModel.settingValue(
                                        forKey: vcKey,
                                        defaultValue: ""))
            }
    }

    @ViewBuilder
    private func menuContent() -> some View {
        let current = rawValue

        if !current.isEmpty {
            Button {
                UIPasteboard.general.string = Self.clipboardPrefix + current
            } label: {
                Label("Copy Value", systemImage: "doc.on.doc")
            }
        }

        // Always present Paste — reading UIPasteboard at menu-build
        // time triggers iOS's pasteboard-access privacy banner even
        // when the user never taps Paste (SwiftUI evaluates
        // `.contextMenu` builders eagerly with the parent view, so
        // populating the inspector for any effect would fire the
        // "xLights would like to paste from…" prompt across every
        // property). The action handler is the only place we touch
        // the pasteboard; if the clipboard doesn't carry our
        // `xlprop:v1:` prefix the tap is a no-op.
        Button {
            guard let s = UIPasteboard.general.string,
                  s.hasPrefix(Self.clipboardPrefix) else {
                return
            }
            let pasted = String(s.dropFirst(Self.clipboardPrefix.count))
            if pasted != current {
                viewModel.setSettingValue(pasted,
                                          forKey: settingKey,
                                          suppressIfDefault: defaultValueString)
            }
        } label: {
            Label("Paste Value", systemImage: "doc.on.clipboard")
        }

        if !isAtDefault {
            Button {
                viewModel.setSettingValue(defaultValueString,
                                          forKey: settingKey,
                                          suppressIfDefault: defaultValueString)
            } label: {
                Label("Reset to Default", systemImage: "arrow.uturn.backward")
            }
        }

        if property.valueCurve == true {
            Divider()
            Button {
                showVCEditor = true
            } label: {
                Label("Edit Value Curve…", systemImage: "chart.xyaxis.line")
            }

            // G37 — serialised VC round-trip via pasteboard. Copy
            // is enabled only when this property actually has a
            // stored curve (active); Paste is enabled when the
            // clipboard carries our `xlvc:v1:` prefix.
            let vcKey = property.valueCurveKey(prefix: metadataPrefix)
            let stored = viewModel.settingValue(forKey: vcKey,
                                                defaultValue: "")
            let hasActiveVC = stored.hasPrefix("Active=TRUE")
            if hasActiveVC {
                Button {
                    UIPasteboard.general.string =
                        ValueCurveClipboard.wrap(stored)
                } label: {
                    Label("Copy Value Curve",
                          systemImage: "doc.on.doc")
                }
            }
            Button {
                // Read at tap time only (see Copy / Paste / Reset
                // above — building the menu eagerly would fire the
                // iOS pasteboard banner on every inspector load).
                guard let payload = ValueCurveClipboard.unwrap(
                    UIPasteboard.general.string) else { return }
                viewModel.setSettingValue(
                    payload,
                    forKey: vcKey,
                    suppressIfDefault: "Active=FALSE|")
            } label: {
                Label("Paste Value Curve",
                      systemImage: "doc.on.clipboard")
            }
        }
    }
}

extension View {
    /// Attaches the standard per-property long-press menu (Copy / Paste /
    /// Reset / Edit Value Curve).
    func propertyContextMenu(property: PropertyMetadata,
                             prefix: String) -> some View {
        modifier(PropertyContextMenu(property: property,
                                     metadataPrefix: prefix))
    }
}
