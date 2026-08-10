import Foundation

/// Show-folder backup engine — reads and writes desktop's backup format
/// (`<show>/Backup/<yyyy-MM-dd>-<HHmmss>[_A..K][_OnStart]/` directories
/// mirroring the show tree), so backups made on either platform are
/// restorable from the other with no conversion. Desktop reference:
/// `xLightsFrame::DoBackup` / `BackupDirectory` (`xLightsMain.cpp`) and
/// `RestoreBackupDialog`. The per-sequence snapshot ring
/// (`SequencerViewModel.writeSaveBackup`) writes flat *files* into the
/// same `Backup/` directory; the two coexist because runs are
/// directories and snapshots are files.
enum ShowFolderBackup {

    /// Desktop's seven copy globs (`BackupDirectory`), as extensions.
    static let backedUpExtensions: Set<String> = ["xsq", "xml", "xbkp", "json", "jbkp", "xmap", "xschedule"]

    /// Desktop's `MAXBACKUPFILE_MB` — larger files are skipped unless
    /// the backup is forced (pre-recovery safety backups are forced).
    static let maxFileBytes: Int64 = 30 * 1024 * 1024

    /// Config files a backup run can restore, in desktop's
    /// `RestoreBackupDialog::PopulateLayoutList` order.
    static let restorableConfigFiles = [
        "xlights_networks.xml",
        "xlights_rgbeffects.xml",
        "xlights_keybindings.xml",
        "xlights_effectpresets.json",
    ]

    struct CreateResult {
        let path: String
        let fileCount: Int
        let errors: [String]
        var runName: String { (path as NSString).lastPathComponent }
    }

    enum BackupError: LocalizedError {
        case noShowFolder
        case cannotCreateDirectory(String)
        case noUniqueName

        var errorDescription: String? {
            switch self {
            case .noShowFolder:
                return "No show folder is loaded."
            case .cannotCreateDirectory(let dir):
                return "Unable to create backup directory \"\(dir)\"."
            case .noUniqueName:
                return "Unable to find a unique name for the backup directory."
            }
        }
    }

    /// One backup run directory under `<show>/Backup/`.
    struct BackupRun: Identifiable, Hashable {
        let path: String
        let name: String
        /// Parsed from the `yyyy-MM-dd-HHmmss` name prefix; nil for
        /// directories that don't follow the naming scheme.
        let date: Date?
        var id: String { path }
        var isOnStart: Bool { name.hasSuffix("_OnStart") }
    }

    private static func timestampFormatter() -> DateFormatter {
        let fmt = DateFormatter()
        // en_US_POSIX so a 12-hour region can't rewrite HH to hh + a
        // localised AM/PM marker (same guard as writeSaveBackup).
        fmt.locale = Locale(identifier: "en_US_POSIX")
        fmt.dateFormat = "yyyy-MM-dd-HHmmss"
        return fmt
    }

    /// Create a new backup run. Mirrors `DoBackup`: a fresh
    /// `Backup/<date>-<time>` directory (collision-suffixed `_A`…`_K`,
    /// `_OnStart` appended for launch-time backups) holding every file
    /// matching the seven globs, recursively, skipping directories
    /// named "Backup" and files over 30 MB unless `forceAllFiles`.
    /// Caller is responsible for having obtained write access to the
    /// show folder first.
    static func createBackup(showFolder: String, forceAllFiles: Bool, onStart: Bool = false) throws -> CreateResult {
        guard !showFolder.isEmpty else { throw BackupError.noShowFolder }
        let fm = FileManager.default
        let backupRoot = (showFolder as NSString).appendingPathComponent("Backup")
        do {
            try fm.createDirectory(atPath: backupRoot, withIntermediateDirectories: true)
        } catch {
            throw BackupError.cannotCreateDirectory(backupRoot)
        }

        let stamp = timestampFormatter().string(from: Date())
        let onStartSuffix = onStart ? "_OnStart" : ""
        var runDir = (backupRoot as NSString).appendingPathComponent(stamp + onStartSuffix)
        var tries = 0
        while fm.fileExists(atPath: runDir) && tries < 11 {
            let letter = String(UnicodeScalar(UInt8(65 + tries)))
            runDir = (backupRoot as NSString).appendingPathComponent(stamp + "_" + letter + onStartSuffix)
            tries += 1
        }
        if tries == 11 { throw BackupError.noUniqueName }

        var errors: [String] = []
        var fileCount = 0
        copyTree(from: showFolder, to: runDir, forceAllFiles: forceAllFiles,
                 fileCount: &fileCount, errors: &errors)
        return CreateResult(path: runDir, fileCount: fileCount, errors: errors)
    }

    /// Recursive mirror of one directory level. The target directory is
    /// created lazily, only once a file actually lands in it, so empty
    /// subfolders aren't mirrored (desktop's `CreateMissingDirectories`).
    private static func copyTree(from sourceDir: String, to targetDir: String,
                                 forceAllFiles: Bool,
                                 fileCount: inout Int, errors: inout [String]) {
        let fm = FileManager.default
        let srcURL = URL(fileURLWithPath: sourceDir, isDirectory: true)
        guard let entries = try? fm.contentsOfDirectory(
            at: srcURL,
            includingPropertiesForKeys: [.isDirectoryKey, .isSymbolicLinkKey, .fileSizeKey],
            options: [.skipsHiddenFiles]) else { return }

        var targetCreated = fm.fileExists(atPath: targetDir)
        for url in entries.sorted(by: { $0.lastPathComponent < $1.lastPathComponent }) {
            let values = try? url.resourceValues(forKeys: [.isDirectoryKey, .isSymbolicLinkKey, .fileSizeKey])
            if values?.isSymbolicLink == true { continue }
            let name = url.lastPathComponent
            if values?.isDirectory == true {
                if name.lowercased() == "backup" { continue }
                copyTree(from: url.path,
                         to: (targetDir as NSString).appendingPathComponent(name),
                         forceAllFiles: forceAllFiles,
                         fileCount: &fileCount, errors: &errors)
                continue
            }
            guard backedUpExtensions.contains(url.pathExtension.lowercased()) else { continue }
            if !forceAllFiles, let size = values?.fileSize, Int64(size) > maxFileBytes {
                continue
            }
            if !targetCreated {
                do {
                    try fm.createDirectory(atPath: targetDir, withIntermediateDirectories: true)
                    targetCreated = true
                } catch {
                    errors.append("Unable to create directory \"\(targetDir)\".")
                    return
                }
            }
            do {
                try fm.copyItem(atPath: url.path,
                                toPath: (targetDir as NSString).appendingPathComponent(name))
                fileCount += 1
            } catch {
                errors.append("Unable to copy file \"\(name)\".")
            }
        }
    }

    /// Enumerate backup runs (directories only — the snapshot ring's
    /// flat files are a different feature), newest first by name; the
    /// timestamp naming makes lexical order chronological.
    static func availableRuns(showFolder: String) -> [BackupRun] {
        guard !showFolder.isEmpty else { return [] }
        let backupRoot = (showFolder as NSString).appendingPathComponent("Backup")
        let fm = FileManager.default
        guard let entries = try? fm.contentsOfDirectory(atPath: backupRoot) else { return [] }
        let fmt = timestampFormatter()
        var runs: [BackupRun] = []
        for name in entries {
            let full = (backupRoot as NSString).appendingPathComponent(name)
            var isDir: ObjCBool = false
            guard fm.fileExists(atPath: full, isDirectory: &isDir), isDir.boolValue else { continue }
            let date = fmt.date(from: String(name.prefix(17)))
            runs.append(BackupRun(path: full, name: name, date: date))
        }
        return runs.sorted { $0.name > $1.name }
    }

    /// Config files present at a run's root, in desktop's restore order.
    static func configFiles(inRun runPath: String) -> [String] {
        let fm = FileManager.default
        return restorableConfigFiles.filter {
            fm.fileExists(atPath: (runPath as NSString).appendingPathComponent($0))
        }
    }

    /// Sequence files in a run, as run-relative paths. Desktop's
    /// `GetSeqList` rule: any `.xsq`/`.xml` whose name doesn't contain
    /// "xlights_", found recursively.
    static func sequenceFiles(inRun runPath: String) -> [String] {
        let fm = FileManager.default
        let runURL = URL(fileURLWithPath: runPath, isDirectory: true)
        guard let walker = fm.enumerator(at: runURL,
                                         includingPropertiesForKeys: [.isDirectoryKey],
                                         options: [.skipsHiddenFiles]) else { return [] }
        var result: [String] = []
        let runComponents = runURL.standardizedFileURL.pathComponents
        for case let url as URL in walker {
            let values = try? url.resourceValues(forKeys: [.isDirectoryKey])
            if values?.isDirectory == true { continue }
            let name = url.lastPathComponent
            let ext = url.pathExtension.lowercased()
            guard ext == "xsq" || ext == "xml" else { continue }
            if name.lowercased().contains("xlights_") { continue }
            let components = url.standardizedFileURL.pathComponents
            guard components.count > runComponents.count else { continue }
            result.append(components[runComponents.count...].joined(separator: "/"))
        }
        return result.sorted()
    }

    /// Copy the selected run-relative files back over the show folder,
    /// creating intermediate directories as needed. Returns per-file
    /// error strings (empty on full success). Caller is responsible for
    /// write access and for reloading show state afterwards.
    static func restore(files: [String], fromRun runPath: String, toShowFolder showFolder: String) -> [String] {
        let fm = FileManager.default
        var errors: [String] = []
        for relative in files {
            let src = (runPath as NSString).appendingPathComponent(relative)
            let dest = (showFolder as NSString).appendingPathComponent(relative)
            do {
                let destDir = (dest as NSString).deletingLastPathComponent
                try fm.createDirectory(atPath: destDir, withIntermediateDirectories: true)
                if fm.fileExists(atPath: dest) {
                    try fm.removeItem(atPath: dest)
                }
                try fm.copyItem(atPath: src, toPath: dest)
            } catch {
                errors.append("Unable to copy file \"\(relative)\".")
            }
        }
        return errors
    }
}
