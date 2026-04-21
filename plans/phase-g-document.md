# Phase G — Document / iCloud polish

Builds on the Phase E document lifecycle. Phase E gets save / save-as
/ open / close / new working against the local filesystem and the
show-folder bookmark; Phase G layers on the iPadOS-idiomatic polish
around iCloud Drive, multi-process file coordination, and the "Open
in xLights" system integration.

1. **File coordination for sequence writes** (`NSFileCoordinator`) —
   Phase E writes `.xsq` directly; this phase wraps writes in a
   coordinator so concurrent Files-app activity can't corrupt.
2. **Ubiquity status** — show "downloading" state for sequences not yet
   materialized locally. `FileExists()` already triggers iCloud
   downloads; surface the progress.
3. **Register `.xsq` as a document type** (`UTExportedTypeDeclarations`)
   so Files app "open in xLights" works.
4. **`Scene.willResignActive`** — pause output, flush render cache,
   stop timers.
