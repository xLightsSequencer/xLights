import Foundation

/// Owns the sequencer's `UndoManager` and counts every change to its
/// history: a new undo group opening (something registered an undo
/// outside an already open group), an undo, or a redo. Code that wants
/// to fold a later edit into the undo it registered itself compares
/// `serial` with the value it saw right after registering; if the
/// serial moved, something else has happened since.
///
/// UndoManager opens a group lazily, on the first registration in a
/// run-loop pass, so idle passes don't move the serial.
@MainActor
final class UndoActivity {
    let manager = UndoManager()
    private(set) var serial = 0
    nonisolated(unsafe) private var tokens: [NSObjectProtocol] = []

    init() {
        let names: [Notification.Name] = [
            .NSUndoManagerDidOpenUndoGroup,
            .NSUndoManagerDidUndoChange,
            .NSUndoManagerDidRedoChange,
        ]
        for name in names {
            tokens.append(NotificationCenter.default.addObserver(
                forName: name, object: manager, queue: nil) { [weak self] _ in
                    // UndoManager is main-actor bound and posts on the main thread.
                    MainActor.assumeIsolated {
                        self?.serial &+= 1
                    }
                })
        }
    }

    deinit {
        for token in tokens {
            NotificationCenter.default.removeObserver(token)
        }
    }
}
