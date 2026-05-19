import UIKit

/// UIAlertController-backed username/password prompt for FPP Connect.
/// Registered with the bridge in `FPPConnectSheet.onAppear`; the
/// bridge invokes `present(host:completion:)` from the main thread
/// when an FPP returns HTTP 401 during discovery / probing, then
/// blocks the calling background thread on a DispatchSemaphore until
/// `completion` fires.
///
/// Stored credentials (Keychain, keyed by IP) are checked first
/// inside the bridge — this prompt only fires when there's no saved
/// match. The "Save Password" toggle defaults on because most users
/// who supply a password want it remembered for the next discovery
/// run.
@MainActor
enum FPPAuthPrompt {
    /// Must be called on the main thread. The bridge ensures this.
    /// `completion` may be called from the main thread (it just
    /// signals a DispatchSemaphore; the bridge waits on it).
    static func present(host: String,
                         completion: @escaping (String?, String?, Bool) -> Void) {
        guard let presenter = topPresenter() else {
            // No window to present from (sheet dismissed mid-flight?
            // App backgrounded?). Treat as cancel so discovery can
            // move on rather than wedging the bridge thread.
            completion(nil, nil, false)
            return
        }

        let alert = UIAlertController(
            title: "Password Required",
            message: "Enter the password for the FPP at \(host).",
            preferredStyle: .alert)

        alert.addTextField { tf in
            tf.placeholder = "Username (defaults to admin)"
            tf.text = "admin"
            tf.autocapitalizationType = .none
            tf.autocorrectionType = .no
        }
        alert.addTextField { tf in
            tf.placeholder = "Password"
            tf.isSecureTextEntry = true
            tf.autocapitalizationType = .none
            tf.autocorrectionType = .no
        }

        // SwitchAction-style "Save Password" — UIAlertController
        // doesn't have a built-in toggle, so we use a third button
        // styled as a checkbox state. iOS users are used to a Save
        // checkbox in keychain prompts; this toggles before the
        // user taps Sign In.
        var savePassword = true
        weak var weakAlert: UIAlertController? = alert
        let saveTitle: () -> String = { savePassword ? "☑ Save Password" : "☐ Save Password" }
        let saveAction = UIAlertAction(title: saveTitle(), style: .default) { _ in
            // We can't reuse the same alert after dismissal, so
            // re-present the same prompt with the toggle flipped.
            // Carry whatever the user already typed.
            let typedUser = weakAlert?.textFields?[0].text ?? "admin"
            let typedPwd = weakAlert?.textFields?[1].text ?? ""
            savePassword.toggle()
            present(host: host,
                     completion: completion,
                     initialUser: typedUser,
                     initialPassword: typedPwd,
                     initialSave: savePassword)
        }
        alert.addAction(saveAction)

        let cancel = UIAlertAction(title: "Cancel", style: .cancel) { _ in
            completion(nil, nil, false)
        }
        alert.addAction(cancel)

        let signIn = UIAlertAction(title: "Sign In", style: .default) { [weak alert] _ in
            let user = alert?.textFields?[0].text ?? "admin"
            let pwd = alert?.textFields?[1].text ?? ""
            completion(user.isEmpty ? "admin" : user, pwd, savePassword)
        }
        alert.addAction(signIn)
        alert.preferredAction = signIn

        presenter.present(alert, animated: true)
    }

    /// Variant used when the user toggled "Save Password" — restores
    /// their typed text + the new toggle state.
    private static func present(host: String,
                                 completion: @escaping (String?, String?, Bool) -> Void,
                                 initialUser: String,
                                 initialPassword: String,
                                 initialSave: Bool) {
        guard let presenter = topPresenter() else {
            completion(nil, nil, false)
            return
        }

        let alert = UIAlertController(
            title: "Password Required",
            message: "Enter the password for the FPP at \(host).",
            preferredStyle: .alert)

        alert.addTextField { tf in
            tf.text = initialUser
            tf.placeholder = "Username"
            tf.autocapitalizationType = .none
            tf.autocorrectionType = .no
        }
        alert.addTextField { tf in
            tf.text = initialPassword
            tf.placeholder = "Password"
            tf.isSecureTextEntry = true
            tf.autocapitalizationType = .none
            tf.autocorrectionType = .no
        }

        var savePassword = initialSave
        weak var weakAlert: UIAlertController? = alert
        let saveTitle: () -> String = { savePassword ? "☑ Save Password" : "☐ Save Password" }
        let saveAction = UIAlertAction(title: saveTitle(), style: .default) { _ in
            let typedUser = weakAlert?.textFields?[0].text ?? "admin"
            let typedPwd = weakAlert?.textFields?[1].text ?? ""
            savePassword.toggle()
            present(host: host,
                     completion: completion,
                     initialUser: typedUser,
                     initialPassword: typedPwd,
                     initialSave: savePassword)
        }
        alert.addAction(saveAction)

        let cancel = UIAlertAction(title: "Cancel", style: .cancel) { _ in
            completion(nil, nil, false)
        }
        alert.addAction(cancel)

        let signIn = UIAlertAction(title: "Sign In", style: .default) { [weak alert] _ in
            let user = alert?.textFields?[0].text ?? "admin"
            let pwd = alert?.textFields?[1].text ?? ""
            completion(user.isEmpty ? "admin" : user, pwd, savePassword)
        }
        alert.addAction(signIn)
        alert.preferredAction = signIn

        presenter.present(alert, animated: true)
    }

    /// Walk the active foreground UIWindowScene to its key window's
    /// topmost presented view controller. Returns nil when the app
    /// has no active scene (background / not yet visible).
    private static func topPresenter() -> UIViewController? {
        let scene = UIApplication.shared.connectedScenes.first { s in
            (s as? UIWindowScene)?.activationState == .foregroundActive
        } as? UIWindowScene
        guard let window = scene?.windows.first(where: { $0.isKeyWindow })
                ?? scene?.windows.first else {
            return nil
        }
        var vc = window.rootViewController
        while let presented = vc?.presentedViewController {
            vc = presented
        }
        return vc
    }
}
