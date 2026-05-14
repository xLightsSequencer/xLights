# Phase G — Document / iCloud polish

**Status: complete (2026-04-22).**

All in-scope Phase G work shipped on top of the Phase E
document lifecycle:

- **G-1** `NSFileCoordinator`-wrapped sequence writes —
  `SequencerViewModel.coordinatedWrite(at:_:)` covers
  `saveSequence`, `saveSequenceAs`, and `tickAutosave`.
- **G-2** iCloud ubiquity status badges — `UbiquityStatus`
  enum + `UbiquityBadge` view in `XLightsApp.swift`, applied
  to Recent and show-folder rows; `.notDownloaded` taps run
  `startDownloadingUbiquitousItem` with a ~5s poll.
- **G-3** `.xsq` + `.xsqz` document registration via
  `macOS/Assets/xLights-iPad/Info.plist` (`org.xlights.sequence`
  and `org.xlights.sequence-package` UTIs, both Editor /
  Owner) + `LSSupportsOpeningDocumentsInPlace` +
  `UIFileSharingEnabled`. `ContentView.onOpenURL` routes
  `.xsq` through `openSequence` (queued in `pendingOpenURL`
  if the show folder isn't loaded yet) and `.xsqz` through
  the sandbox round-trip.
- **G-4** background lifecycle — `quiesceForInactive` pauses
  playback / scrub; `shutdownForBackground` cancels render
  polling, calls `abortRenderAndWait(3.0)`, and stops
  controller output (iOS throttles background traffic so
  sACN/ArtNet/DDP becomes unreliable).
- **`.xsqz` round-trip.** Open: `copyPackageToSandbox` uses
  `NSFileCoordinator.coordinate(readingItemAt:)` + URL-based
  `FileManager.copyItem` to stage the package inside
  `NSTemporaryDirectory()` — iOS path-based POSIX syscalls
  can't read `~/Library/Mobile Documents` without iCloud
  entitlements, so all subsequent path-based operations run
  against the sandbox copy. Bridge
  `openPackagedSequence(atPath:)` extracts via
  `SequencePackage` and `LoadShowFolder(GetTempShowFolder())`
  (handles old-format packages that nest under
  `<showname>/`). Save: bridge `saveSequence` repacks via
  `SequencePackage::Pack`; view model
  `copySandboxBackToOriginal` writes back through
  `NSFileCoordinator.coordinate(writingItemAt:.forReplacing)`
  with URL-based copy. Atomic at both ends. Close: package
  destructor wipes the extraction temp dir, previous show
  folder restored if one was configured.
- **Shared `SequencePackage::Pack`** in `src-core/render/`
  replaces the wx-only `xLightsFrame::PackageSequence`. Walks
  `SequenceMedia.GetAllMediaPaths()`, every model /
  view-object's `GetFileReferences()`, and Matrix-face images
  via `Model::GetFaceFiles(all:true)`. Show-relative paths
  preserved, externals relocated under typed subdirs
  (`Images/`, `Videos/`, `Shaders/`, `Glediators/`, `Faces/`,
  `Objects/`), basename collisions disambiguate by subfolder
  (so `PicturesEffect` doesn't misread numeric suffixes as
  animation-sequence markers), per-object group colocation
  for mesh + `.mtl` + textures, pre-flight readability check,
  per-file warnings collected to `outWarnings` (desktop
  surfaces them via `DisplayWarning`), atomic rename. Desktop
  `PackageSequence` now delegates entirely to `Pack()`.
  Font bundling for Text-effect packages added 2026-04-26
  (`a8f9087bb`) — flows automatically to iPad since both
  clients call the same `Pack()`.
- **Preferences rename** — "Exclude Presets" (a no-op) renamed
  to "Exclude Videos", which addresses the legitimate
  copyright concern that motivated the original name.

## Deferred

- `.piz` and bare `.zip` UTIs intentionally not registered —
  routing every arbitrary `.zip` tap into xLights would be
  hostile. `SequencePackage` still handles them in-core when
  the user picks one explicitly.

## Caveats

- Save-back for `.xsqz` opened from non-Files-provider sources
  (e.g. deep-linked via `itms-services://`) is untested but
  should work through the same sandbox round-trip.
- Fresh-install tap on a `.xsqz` opens straight to the
  sequencer with no show-folder configuration; close flips
  `isShowFolderLoaded` back to false so the user returns to
  the setup prompt.

## Post-completion hardening

- **Show-folder load detached from main actor (2026-05-14).**
  Crash triage of 2026.08 surfaced 27+ `0x8BADF00D` watchdog
  kills across two builds, all bottoming out in
  `MeshObject::checkAccessToFile → FileExists` (iCloud
  download) or `ObtainAccessToURL` (security-scoped bookmark
  resolution) — both reached via the synchronous chain
  `FolderConfigView.apply()` → `SequencerViewModel.loadShowFolder`
  → `iPadRenderContext::LoadShowFolder`, and again on launch
  via `restorePersistedShowFolder()`. `loadShowFolder` now
  runs the obtainAccess pre-flight, the C++ load, and the
  recursive `.xsq` scan on `Task.detached`, hopping back to
  `@MainActor` only to apply `isShowFolderLoaded` and the
  resulting `sequenceFiles` array. Callers that previously
  relied on `restorePersistedShowFolder()` returning the
  load result synchronously must instead observe
  `viewModel.isShowFolderLoaded` (the existing `.onChange`
  call sites already do this; only the return-value semantics
  changed).
