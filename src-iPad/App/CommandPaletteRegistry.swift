import SwiftUI

// Fuzzy subsequence matcher — approximates desktop's
// `CommandPaletteDialog::FuzzyMatch` / `GetFuzzyMatchPositions`:
//   substring hit scores highest (prefix > mid-string), otherwise an
//   in-order subsequence match scores by adjacency + word-boundary
//   bonuses. Case-insensitive throughout.
enum CommandPaletteMatch {
    /// Score `text` against `query`, or nil when `query` is not a
    /// subsequence of `text`. Higher is better.
    static func score(_ text: String, query: String) -> Int? {
        if query.isEmpty { return 0 }
        let lt = Array(text.lowercased())
        let lp = Array(query.lowercased())

        if let r = String(lt).range(of: String(lp)) {
            return String(lt).distance(from: String(lt).startIndex,
                                       to: r.lowerBound) == 0 ? 100 : 50
        }

        var p = 0
        var s = 0
        var prevMatched = false
        for (i, ch) in lt.enumerated() where p < lp.count {
            if ch == lp[p] {
                p += 1
                s += prevMatched ? 5 : 1
                if i == 0 || " _->/-".contains(lt[i - 1]) { s += 10 }
                prevMatched = true
            } else {
                prevMatched = false
            }
        }
        return p == lp.count ? s : nil
    }

    /// Character offsets in `text` that matched `query` — for inline
    /// highlighting. Empty when there's no match.
    static func positions(in text: String, query: String) -> [Int] {
        if query.isEmpty { return [] }
        let lt = Array(text.lowercased())
        let lp = Array(query.lowercased())

        if let r = String(lt).range(of: String(lp)) {
            let start = String(lt).distance(from: String(lt).startIndex, to: r.lowerBound)
            return Array(start..<(start + lp.count))
        }

        var out: [Int] = []
        var p = 0
        for (i, ch) in lt.enumerated() where p < lp.count {
            if ch == lp[p] {
                out.append(i)
                p += 1
            }
        }
        return p == lp.count ? out : []
    }
}

// Static command registry mirroring the menu items in
// `XLightsCommands.swift`. Each entry routes through the same
// `viewModel` calls / tokens the menu buttons use and carries the
// same enabled-gate, so a disabled command is filtered out (desktop's
// `CommandPaletteDialog` skips `!IsEnabled()` items rather than greying
// them — see `CollectMenuItems`). Items needing `openWindow` /
// `dismissWindow` Environment (preview/inspector detach, layout editor)
// are intentionally omitted — they're iPad scene idioms without a
// desktop menu-command equivalent and can't run from a detached
// closure.
@MainActor
enum CommandPaletteRegistry {
    static func commands(viewModel vm: SequencerViewModel,
                         timeline: TimelineState?) -> [PaletteCommand] {
        let loaded = vm.isSequenceLoaded
        let hasSel = vm.selectedEffect != nil
        let hasAnySel = vm.selectedEffect != nil || !vm.selectedEffects.isEmpty
        var out: [PaletteCommand] = []

        func add(_ name: String, _ cat: String, _ sym: String,
                 _ sc: String, _ enabled: Bool, _ action: @escaping () -> Void) {
            if enabled { out.append(PaletteCommand(name: name, category: cat,
                                                   shortcut: sc, symbol: sym,
                                                   action: action)) }
        }

        // File
        add("Save", "File", "square.and.arrow.down", "⌘S", vm.isDirty) {
            _ = vm.saveSequence()
        }
        add("Save As…", "File", "square.and.arrow.down.on.square", "⇧⌘S", loaded) {
            vm.saveAsRequestToken &+= 1
        }
        add("Close", "File", "xmark.circle", "⌘W", loaded) {
            vm.closeRequestToken &+= 1
        }
        add("Sequence Settings…", "File", "gearshape", "", loaded) {
            vm.showingSequenceSettings = true
        }

        // Edit
        add("Undo", "Edit", "arrow.uturn.backward", "⌘Z", vm.undoManager.canUndo) {
            vm.undo()
        }
        add("Redo", "Edit", "arrow.uturn.forward", "⇧⌘Z", vm.undoManager.canRedo) {
            vm.redo()
        }
        add("Copy", "Edit", "doc.on.doc", "⌘C", hasSel) {
            vm.copySelectedEffect()
        }
        add("Paste", "Edit", "doc.on.clipboard", "⌘V", vm.hasClipboard) {
            let rowIdx = vm.selectedEffect?.rowIndex
                ?? vm.rows.firstIndex(where: { $0.timing == nil }) ?? 0
            vm.pasteEffect(rowIndex: rowIdx, startMS: vm.playPositionMS)
        }
        add("Duplicate", "Edit", "plus.square.on.square", "⌘D", hasSel) {
            vm.duplicateSelectedEffect()
        }
        add("Select All", "Edit", "checklist", "⌘A", vm.hasAnyEffectToSelect) {
            vm.selectAllEffects()
        }
        add("Delete", "Edit", "trash", "⌫", hasAnySel) {
            vm.deleteSelectedEffects()
        }
        add("Clear Selection", "Edit", "rectangle.dashed", "esc", hasAnySel) {
            vm.clearSelection()
        }
        add("Find / Replace…", "Edit", "magnifyingglass", "⌘F", loaded) {
            vm.findReplacePresented = true
        }
        add("Color Replace…", "Edit", "paintpalette", "", loaded) {
            vm.colorReplacePresented = true
        }
        add("Effect Presets…", "Edit", "star", "⇧⌘P", loaded) {
            vm.presetBrowserPresented = true
        }

        // View
        add(vm.showPreview ? "Hide Preview" : "Show Preview", "View",
            "rectangle.on.rectangle", "⌘1", loaded) {
            vm.togglePreview()
        }
        add(vm.showInspector ? "Hide Inspector" : "Show Inspector", "View",
            "sidebar.right", "⌘2", loaded) {
            vm.showInspector.toggle()
        }
        add("Zoom In", "View", "plus.magnifyingglass", "⌘=", timeline != nil) {
            timeline?.pixelsPerMS = min(2.0, (timeline?.pixelsPerMS ?? 0.01) * 1.5)
        }
        add("Zoom Out", "View", "minus.magnifyingglass", "⌘-", timeline != nil) {
            timeline?.pixelsPerMS = max(0.005, (timeline?.pixelsPerMS ?? 0.01) / 1.5)
        }
        add("Edit Display Elements…", "View", "list.bullet.rectangle", "⇧⌘D", loaded) {
            vm.showingDisplayElements = true
        }

        // Tools
        add("Import Effects…", "Tools", "square.and.arrow.down.on.square", "", loaded) {
            vm.showingImportEffects = true
        }
        add("Check Sequence…", "Tools", "checkmark.seal", "", loaded) {
            vm.showingCheckSequence = true
        }
        add("AI Services…", "Tools", "wand.and.stars", "", true) {
            vm.showingAIServices = true
        }
        add("Package Logs…", "Tools", "doc.zipper", "", true) {
            Task { @MainActor in
                if let url = await vm.packageLogs() { XLPresentShareSheet(items: [url]) }
            }
        }
        add("Package Sequence…", "Tools", "shippingbox", "", loaded) {
            vm.showingPackageSequence = true
        }
        add("Export House Preview…", "Tools", "film", "", loaded) {
            vm.showingExportHousePreview = true
        }
        add("FPP Connect…", "Tools", "antenna.radiowaves.left.and.right", "",
            vm.isShowFolderLoaded) {
            vm.showingFPPConnect = true
        }
        add("View Log…", "Tools", "text.alignleft", "", true) {
            vm.showingLogViewer = true
        }
        add("Purge Render Cache", "Tools", "trash.slash", "", loaded) {
            vm.purgeRenderCache()
        }
        add("Purge Download Cache", "Tools", "trash.slash", "", true) {
            vm.purgeDownloadCache()
        }

        // Playback
        add(vm.isPlaying ? "Pause" : "Play", "Playback",
            vm.isPlaying ? "pause.fill" : "play.fill", "space", loaded) {
            vm.togglePlayPause()
        }
        add("Stop", "Playback", "stop.fill", "", loaded) { vm.stop() }
        add("Render All", "Playback", "arrow.triangle.2.circlepath", "⌘R", loaded) {
            vm.startBackgroundRender()
        }
        add("Rewind to Start", "Playback", "backward.end.fill", "home", loaded) {
            vm.seekTo(ms: 0)
        }
        add("Jump to End", "Playback", "forward.end.fill", "end", loaded) {
            vm.seekTo(ms: vm.sequenceDurationMS)
        }
        add("Back 10 Seconds", "Playback", "gobackward.10", "", loaded) {
            vm.seekTo(ms: max(0, vm.playPositionMS - 10_000))
        }
        add("Forward 10 Seconds", "Playback", "goforward.10", "", loaded) {
            vm.seekTo(ms: min(vm.sequenceDurationMS, vm.playPositionMS + 10_000))
        }
        for opt in XLPlaybackSpeeds.options {
            add("Playback Speed \(opt.label)", "Playback", "speedometer", "", loaded) {
                vm.setPlaybackSpeed(opt.rate)
            }
        }
        add("Clear All Tags", "Playback", "tag.slash", "",
            loaded && vm.tagPositions.contains(where: { $0 >= 0 })) {
            vm.clearAllTags()
        }
        for i in 0..<10 where (vm.tagPositions[safe: i] ?? -1) >= 0 {
            add("Go To Tag \(i)", "Playback", "tag", "⌃\(i)", loaded) {
                vm.goToTag(i)
            }
        }

        // Help
        add("About xLights…", "Help", "info.circle", "", true) {
            vm.showingAbout = true
        }
        add("xLights Manual", "Help", "book", "", true) {
            XLOpenURL("https://manual.xlights.org/")
        }
        add("Tutorial Videos", "Help", "play.rectangle", "", true) {
            XLOpenURL("https://videos.xlights.org")
        }
        add("xLights Forum", "Help", "bubble.left.and.bubble.right", "", true) {
            XLOpenURL("https://nutcracker123.com/forum/")
        }
        add("Issue Tracker", "Help", "ladybug", "", true) {
            XLOpenURL("https://github.com/xLightsSequencer/xLights/issues")
        }
        add("xLights Website", "Help", "globe", "", true) {
            XLOpenURL("https://xlights.org")
        }
        add("Donate…", "Help", "heart", "", true) {
            XLOpenURL("https://www.paypal.com/donate/?hosted_button_id=BB6366BT755H6")
        }

        return out
    }
}

private extension Array {
    subscript(safe index: Int) -> Element? {
        indices.contains(index) ? self[index] : nil
    }
}
