import SwiftUI
import UIKit

/// Help → About xLights. Mirrors the desktop About dialog
/// (`src-ui-wx/app-shell/AboutDialog.cpp`):
/// app icon, version + build, the GPL legal text shared with
/// desktop via `XLSequenceDocument.licenseText`, and links for
/// Privacy Policy + EULA. Reachable from the menu bar regardless
/// of whether a sequence is loaded.
struct AboutSheet: View {
    @Environment(\.dismiss) private var dismiss

    private var displayVersion: String {
        XLSequenceDocument.appVersion()
    }

    private var buildNumber: String {
        Bundle.main.infoDictionary?["CFBundleVersion"] as? String ?? "?"
    }

    private var bundleVersion: String {
        // Belt + suspenders: prefer the C++ symbol so we match the
        // string the sequence file marks itself with on save, but
        // fall back to the bundle if the bridge somehow returns
        // empty (unit-test contexts, etc.).
        let v = displayVersion
        return v.isEmpty
            ? (Bundle.main.infoDictionary?["CFBundleShortVersionString"] as? String ?? "?")
            : v
    }

    var body: some View {
        NavigationStack {
            ScrollView {
                VStack(spacing: 20) {
                    appIcon
                    VStack(spacing: 6) {
                        Text("xLights")
                            .font(.largeTitle.weight(.semibold))
                        Text("Version \(bundleVersion) (\(buildNumber))")
                            .font(.subheadline)
                            .foregroundStyle(.secondary)
                            .textSelection(.enabled)
                    }
                    Text(XLSequenceDocument.licenseText())
                        .font(.footnote)
                        .multilineTextAlignment(.leading)
                        .foregroundStyle(.secondary)
                        .textSelection(.enabled)
                        .frame(maxWidth: 560)
                        .padding(.horizontal)
                    VStack(spacing: 12) {
                        Link("Privacy Policy",
                             destination: URL(string: "https://xlights.org/privacy-policy/")!)
                        Link("End User License Agreement",
                             destination: URL(string: "http://kulplights.com/xlights/eula.html")!)
                    }
                    .padding(.bottom, 16)
                }
                .padding(.vertical, 24)
                .frame(maxWidth: .infinity)
            }
            .navigationTitle("About xLights")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    Button("Done") { dismiss() }
                }
            }
        }
    }

    /// Reads the AppIcon asset for display. Falls back to the SF
    /// Symbols light-bulb if the bundled icon can't be resolved
    /// (e.g. during a stripped-down debug build).
    @ViewBuilder
    private var appIcon: some View {
        if let icon = AboutSheet.loadAppIcon() {
            Image(uiImage: icon)
                .resizable()
                .aspectRatio(contentMode: .fit)
                .frame(width: 128, height: 128)
                .clipShape(RoundedRectangle(cornerRadius: 24, style: .continuous))
                .shadow(radius: 4, y: 2)
        } else {
            Image(systemName: "lightbulb.fill")
                .resizable()
                .aspectRatio(contentMode: .fit)
                .frame(width: 96, height: 96)
                .foregroundStyle(.yellow)
        }
    }

    /// Pulls the primary AppIcon variant out of Assets.car. iPadOS
    /// doesn't expose the .icon Liquid Glass bundle directly, so we
    /// fall back through the bundle's icon dictionary.
    private static func loadAppIcon() -> UIImage? {
        // Try the Icon Composer bundle name first.
        if let img = UIImage(named: "AppIcon") { return img }
        // Then the conventional Info.plist icon files.
        guard let icons = Bundle.main.infoDictionary?["CFBundleIcons"] as? [String: Any],
              let primary = icons["CFBundlePrimaryIcon"] as? [String: Any],
              let files = primary["CFBundleIconFiles"] as? [String],
              let last = files.last
        else { return nil }
        return UIImage(named: last)
    }
}

#Preview {
    AboutSheet()
}
