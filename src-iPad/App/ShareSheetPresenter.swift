import UIKit

// Presents a `UIActivityViewController` from whatever view controller
// is topmost on the foreground active scene. SwiftUI menu commands
// don't have an obvious anchor view, so the share sheet has to find
// its own presenter — same pattern PreviewPaneView uses for its
// "share preview snapshot" path.
//
// On iPad the share sheet is a popover; we anchor it at the centre
// of the key window so it has somewhere to point. On iPhone the
// presentation style ignores the source rect.
@MainActor
func XLPresentShareSheet(items: [Any],
                         applicationActivities: [UIActivity]? = nil) {
    let scene = UIApplication.shared.connectedScenes
        .compactMap { $0 as? UIWindowScene }
        .first(where: { $0.activationState == .foregroundActive })
        ?? UIApplication.shared.connectedScenes
            .compactMap { $0 as? UIWindowScene }.first
    guard let window = scene?.keyWindow ?? scene?.windows.first,
          var presenter = window.rootViewController else { return }
    while let next = presenter.presentedViewController {
        presenter = next
    }

    let vc = UIActivityViewController(activityItems: items,
                                       applicationActivities: applicationActivities)
    if let popover = vc.popoverPresentationController {
        popover.sourceView = window
        popover.sourceRect = CGRect(x: window.bounds.midX,
                                    y: window.bounds.midY,
                                    width: 0, height: 0)
        popover.permittedArrowDirections = []
    }
    presenter.present(vc, animated: true)
}

/// Opens `string` in the system browser. Used by the Help menu so
/// each external link mirrors desktop's `wxLaunchDefaultBrowser`.
/// Silently no-ops when the string isn't a valid URL — Help-menu
/// destinations are hard-coded so this can only fire on a typo.
@MainActor
func XLOpenURL(_ string: String) {
    guard let url = URL(string: string) else { return }
    UIApplication.shared.open(url)
}
