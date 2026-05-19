import Foundation
import Observation

/// Drives a serial open → render → write fseq → close loop over a list of
/// sequences. Bypasses `SequencerViewModel.openSequence` / `closeSequence`
/// so we don't mount the sequencer UI between iterations — the picker stays
/// on screen and only the batch sheet observes this runner's state.
///
/// The fseq write is unconditional regardless of `FolderConfig.fseqEnabled`
/// because the user explicitly invoked Batch Render — the whole point is
/// to refresh fseqs against the current layout. `FolderConfig.fseqFolder`
/// is still consulted so a configured fseq directory wins; otherwise the
/// fseq lands next to the .xsq.
@MainActor
@Observable
final class BatchRenderRunner {
    enum Phase: Equatable {
        case idle
        case running(currentIndex: Int, total: Int, currentName: String)
        case done(rendered: Int, failed: Int, cancelled: Bool)
    }

    private(set) var phase: Phase = .idle

    /// Fraction of the *current* sequence's render that is complete (0..1).
    /// Resets to 0 each time we begin a new sequence and rises as the
    /// render workers chew through frames. Distinct from the .running
    /// phase's `currentIndex / total`, which is the across-sequences count.
    private(set) var currentSequenceProgress: Double = 0.0

    private let document: XLSequenceDocument
    private var task: Task<Void, Never>?
    private var cancelRequested: Bool = false

    init(document: XLSequenceDocument) {
        self.document = document
    }

    func start(entries: [SequenceEntry]) {
        guard task == nil, !entries.isEmpty else { return }
        cancelRequested = false
        var rendered = 0
        var failed = 0
        let total = entries.count

        task = Task { @MainActor in
            for (i, entry) in entries.enumerated() {
                if cancelRequested { break }
                currentSequenceProgress = 0.0
                phase = .running(currentIndex: i + 1,
                                 total: total,
                                 currentName: entry.displayName)
                // Hand control back to the runloop so SwiftUI actually
                // commits this phase change *before* we head into the
                // synchronous bridge work in renderOne(). Without this
                // yield, openSequence() blocks the main thread for
                // hundreds of ms (or longer for big sequences) and the
                // sheet appears frozen on the previous frame.
                await Task.yield()

                let ok = await renderOne(entry: entry)
                if cancelRequested { break }
                if ok { rendered += 1 } else { failed += 1 }
            }
            phase = .done(rendered: rendered,
                          failed: failed,
                          cancelled: cancelRequested)
            task = nil
        }
    }

    /// Cancel the in-flight batch. Aborts the current render synchronously
    /// (so the next bridge call doesn't race with a still-running render)
    /// and lets the loop fall through to publish the .done phase.
    func cancel() {
        cancelRequested = true
        _ = document.abortRenderAndWait(5.0)
    }

    /// Reset the runner back to .idle so the sheet can be dismissed and
    /// re-presented for a fresh selection. Caller is responsible for
    /// ensuring no batch is currently running.
    func reset() {
        guard task == nil else { return }
        phase = .idle
        cancelRequested = false
    }

    /// Returns true if we wrote a fresh fseq for this sequence; false on
    /// open / write failure. Logs but doesn't propagate errors — the loop
    /// continues to the next entry on failure.
    ///
    /// Memory-pressure handling: the iOS dispatch source can fire
    /// `SignalAbort` mid-render, which makes `isRenderDone()` flip to
    /// true with `SequenceData` in a partially-populated state. Writing
    /// that to fseq produces invalid playback. We watch for that case
    /// via `wasRenderAborted` and retry the render once after a brief
    /// delay (gives the pressure source time to clear and the cache
    /// purge to take effect). A second abort is a hard fail: we skip
    /// the fseq write rather than persist garbage.
    private func renderOne(entry: SequenceEntry) async -> Bool {
        _ = XLSequenceDocument.obtainAccess(toPath: entry.fullPath,
                                             enforceWritable: true)
        if !document.openSequence(entry.fullPath) {
            print("BatchRender: openSequence failed for \(entry.fullPath)")
            return false
        }

        var attempt = 0
        let maxAttempts = 2
        while attempt < maxAttempts {
            attempt += 1
            if cancelRequested { break }

            // `renderAll()` is non-blocking — it builds the render tree and
            // queues the per-row jobs onto the JobPool, then returns. The
            // actual frame work runs on the pool's background workers. So
            // we call it synchronously on the main thread (cheap, just
            // setup) and then poll isRenderDone for the queued jobs to
            // finish. Spawning a worker thread to call renderAll() is a
            // race: if the poll fires before the thread enters Render(),
            // the RenderProgressInfo list is empty and isRenderDone()
            // reports `true` — we'd write the un-rendered SequenceData and
            // produce an empty fseq.
            currentSequenceProgress = 0.0
            document.renderAll()

            // Poll the bridge's render-done flag. 250ms matches
            // SequencerViewModel.beginFreshRender's cadence — fast enough that
            // a quick render doesn't sit idle, slow enough not to thrash the
            // main runloop. Cancel checks ride the same wakeup. Progress
            // fraction is updated each tick so the sheet's per-sequence bar
            // moves as workers chew through frames.
            while !document.isRenderDone() {
                if cancelRequested {
                    _ = document.abortRenderAndWait(5.0)
                    document.closeSequence()
                    return false
                }
                currentSequenceProgress = Double(document.renderProgressFraction())
                try? await Task.sleep(nanoseconds: 250_000_000)
            }

            if !document.wasRenderAborted() {
                currentSequenceProgress = 1.0
                break
            }

            // Render aborted before completing — almost always memory
            // pressure. Don't persist the partial buffer. If we have
            // attempts left, give the system ~500ms for the pressure
            // source to clear (and `HandleMemoryWarning` has already
            // purged caches) then retry.
            print("BatchRender: render aborted for \(entry.fullPath) on attempt \(attempt)/\(maxAttempts); \(attempt < maxAttempts ? "retrying" : "giving up")")
            if attempt < maxAttempts {
                try? await Task.sleep(nanoseconds: 500_000_000)
            }
        }

        // After the retry loop: if the last attempt is still aborted,
        // skip the fseq write. The .xsq itself isn't being written here,
        // so the user's source is untouched.
        if document.wasRenderAborted() {
            print("BatchRender: skipping fseq write for \(entry.fullPath) — render aborted twice")
            _ = document.abortRenderAndWait(5.0)
            document.closeSequence()
            return false
        }

        let fseqPath = batchRenderFseqPath(forXsq: entry.fullPath)
        _ = XLSequenceDocument.obtainAccess(toPath: fseqPath,
                                             enforceWritable: true)
        let wrote = document.writeFseq(toPath: fseqPath)
        if !wrote {
            print("BatchRender: writeFseq failed for \(fseqPath)")
        }

        // Defensive — the render *should* already be done, but make sure
        // no stragglers reference SequenceData when closeSequence frees it.
        _ = document.abortRenderAndWait(5.0)
        document.closeSequence()
        return wrote
    }
}

/// Resolve the on-disk fseq path for a sequence in batch-render context.
/// Always returns a path (unconditional) — the user explicitly requested a
/// re-render so we always write the fseq. Honors `FolderConfig.fseqFolder`
/// when set; otherwise places the fseq next to the .xsq so subfolders are
/// preserved automatically.
func batchRenderFseqPath(forXsq xsqPath: String) -> String {
    let xsqURL = URL(fileURLWithPath: xsqPath)
    let basename = xsqURL.deletingPathExtension().lastPathComponent + ".fseq"
    if let folder = FolderConfig.fseqFolder {
        return URL(fileURLWithPath: folder).appendingPathComponent(basename).path
    }
    return xsqURL.deletingLastPathComponent().appendingPathComponent(basename).path
}
