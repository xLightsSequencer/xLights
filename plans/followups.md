# Cross-phase follow-ups

Small items left over from phases that otherwise landed. No new phase
home; catalogued here so they don't fall off.

## Phase A — Core-path hardening

- **Re-prompt on failed `ObtainAccessToURL`.** Desktop re-prompts the user
  with `UIDocumentPickerViewController` when a stale security-scoped
  bookmark fails to resolve; iPad currently ignores the return value, so
  a stale bookmark leads to silent lookup failure. Minimum version:
  check the return, log, and drop the failed folder from `_mediaFolders`
  before handing it to `FileUtils`. Full re-prompt UX needs a Swift
  callback + `UIDocumentPickerViewController` hook.

## App lifecycle — energy management (landed 2026-04-20)

When the scene moves to `.inactive` (multitasking switcher, incoming
call, Control Center) `SequencerViewModel.quiesceForInactive()` pauses
playback (preserving play position) and stops scrub so the 30fps frame
timers, audio playback, and preview `CADisplayLink`s (which key off
`isPlaying` / `isScrubbing`) all stop burning energy. The same hook
fires from `.background` via `shutdownForBackground()`, which also
aborts in-flight render workers so a pending termination doesn't race
with `SequenceElements` / `SequenceData` teardown. Animated thumbnails
in the media picker cancel their cycle timer on scene-phase change.
