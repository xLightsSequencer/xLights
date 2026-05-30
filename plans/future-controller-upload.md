# Future — Controller Upload + Pixel Test

The Layout Editor → Controllers tab (J-31 / J-32, 2026-05-16) covers
controller list / edit / discover / wiring-visualize. This file
tracks the **upload** and **test** stack — the Tools-menu entries
that talk to physical controllers (FPP Connect, Bulk Upload,
HinksPix, Pixel Test) plus the heavy port-mapping authoring
dialog and the RemapDMX helper.

The list-management polish that sits on top of the Layout Editor
Controllers tab itself (drag-reorder, sort modes, right-click
Activate/Inactivate, HTTP scan + auth, LED ping) is tracked in
[`phase-j-layout-editor.md`](phase-j-layout-editor.md).

## Status

**FPP Connect (EX-4) — feature-complete for iPad scope**
(2026-05-19). Slices A + B + F + auth-portion of C shipped:
discovery, password-protected FPP support (Keychain-backed
credentials, UIAlertController prompt on 401), per-instance
config (Media / Cape / Add Proxies / Models / UDP Out / Playlist),
parallel transcode across targets, concurrent curl transfers
driven by `CurlManager`, per-FPP progress gauges, full per-FPP
restart / channel-range / playlist-finalize dance mirroring
desktop. iPad targets FPP / ESPixelStick (open-source firmware
only); proprietary controllers (Falcon V4/V5 / Genius / PowerDMX)
are filtered out of discovery.

Remaining Slice C items (manual Add FPP, Re-Discover, sort/right-
click menus) and Slices D / G / H are polish — no concrete user
need is blocking them today. Slice E (proprietary codecs) is
parked indefinitely.

## Slice breakdown

### Slice A (shipped 2026-05-18, but unshippable on its own)

`src-iPad/App/FPPConnectSheet.swift` + bridge in
`XLSequenceDocument.{h,mm}`. Methods:
`discoverFPPInstances` (returns ip/host/version/mode/uuid/fppType),
`uploadFseq:mediaPath:type:toIPAddress:progress:` (only
`PrepareUploadSequence` → `AddFrameToUpload` → `FinalizeUploadSequence`,
mediaPath always nil from Swift, type always 0), `releaseFPPInstances`,
`XLFPPUploadProgress` protocol. Sequential, no parallelism.

### Slice B — Per-instance config UI (the actually-needed work)

Each row in the desktop dialog has 13 columns; iPad has 1. This
slice closes the upload-config gap. **All P1** unless noted.

**Status (2026-05-19):**
- **B1 shipped** (2026-05-18). Discovery filter to FPP +
  ESPixelStick only, automatic fseq codec selection
  (V2-Sparse-zstd for FPP, V2-Sparse-uncompressed for ESPixelStick),
  per-instance Media toggle wired through, `mediaPath(forXsq:)`
  bridge.
- **B2 shipped** (2026-05-19). Per-row chevron-disclosure UI with
  the full settings drawer (Media + Cape + Add Proxies + Models
  + UDP Out + Playlist). Codable `FPPInstanceConfig` persisted
  per-UUID via `FPPInstanceConfigStore` (B1's media-only store
  auto-migrates on first read). FPP `playlists` + `capeModel`
  surfaced in the discovery bridge; Playlist picker shows real
  options + "Create New…" prompt; Cape toggle hides entirely when
  the FPP reports no cape (mirrors desktop's empty-`m`-string
  hide at FPPConnectDialog.cpp:705-707). Also fixed multi-window
  sheet hosting — Tools menu now calls `openWindow("sequencer")`
  before flipping the flag.
- **B3 shipped** (2026-05-19). Cape + Add Proxies wired through
  to the FPP. New bridge surface: `applyConfig(toFPP:settings:progress:)`
  runs `UploadControllerProxies` then (if a single `ControllerEthernet`
  matches the IP) `UploadPanelOutputs` + `UploadVirtualMatrixOutputs`
  + `UploadPixelOutputs` + `UploadSerialOutputs`, followed by
  `Restart(true)` (ifNeeded). `updateChannelRanges(forFPP:)`
  refreshes channel maps post-restart. Runner gained a
  `.configuring` phase that runs per-FPP before the sequence
  upload loop and skips FPPs whose config upload failed. Mirrors
  desktop order at FPPConnectDialog.cpp:1148-1212. ESPixelStick
  targets short-circuit applyConfig (those uploads don't apply).
- **B4 shipped** (2026-05-19). Models + UDP Out + Playlist now
  flow through `applyConfig`. Models "all" runs
  `CreateModelMemoryMap(0..INT32_MAX)` + `UploadModels` +
  `UploadDisplayMap` (built from `CreateVirtualDisplayMap` using
  `_context->GetAllObjects()` + `GetPreviewWidth/Height`) +
  `SetRestartFlag(true)`; "local" restricts to the matched
  controller's start/end channel range. UDP "all" builds the
  universe file from `om.GetControllers()` and pushes via
  `UploadUDPOut`, feeding `udpRanges` back into the FPP's
  `FillRanges`/`SetNewRanges`. UDP "proxied" uses
  `UploadUDPOutputsForProxy`. Playlist's pre-sequence setup pass
  runs inside `applyConfig`. New `finalize(fpp:playlist:)` bridge
  runs the post-sequence `UploadPlaylist` (commits sequences into
  the playlist) + final `Restart(true)`. Runner gained a
  `.finalizing` phase. **Slice B is complete** — all six
  per-instance settings now actually configure the FPP.

| Column | Type | Drives | desktop ref |
|---|---|---|---|
| FSEQ Type | dropdown | `PrepareUploadSequence` `type` arg. Per-fppType choice mapping (FPP: 0=V1, 1=V2-zstd, 2=V2-Sparse-zstd, 3=V2-Sparse-uncompressed; FALCONV4V5/POWERDMX: 0=V1, 1→5 zlib, 2→6 sparse-zlib, 3=sparse-uncompressed, 4=uncompressed; ESPIXELSTICK/Genius: fixed 3). **Without this, iPad uploads broken fseqs.** | `FPPConnectDialog.cpp:593-614, 1242-1252` |
| Media | checkbox | `PrepareUploadSequence`'s `media` arg (currently always ""); plus `CheckUploadMedia` validation. | `FPPConnectDialog.cpp:1237-1240` |
| Models | dropdown (None/All/Local) | `CreateModelMemoryMap(allModels, 0, max)` (All) or `CreateModelMemoryMap(controllerStart, controllerEnd)` (Local) → `UploadModels()` → `UploadDisplayMap()` → `SetRestartFlag(true)`. | `FPPConnectDialog.cpp:1181-1195`, `FPP.cpp UploadModels/CreateModelMemoryMap` |
| UDP Out | dropdown (None/All/Proxied) | "All": `CreateUniverseFile(allControllers, false)` → `UploadUDPOut()` → `SetRestartFlag()`. "Proxied": `UploadUDPOutputsForProxy(outputManager)` → `SetRestartFlag()`. | `FPPConnectDialog.cpp:1156-1168` |
| Add Proxies | checkbox | `UploadControllerProxies(outputManager)`. | `FPPConnectDialog.cpp:1153-1154` |
| Pixel Hat/Cape | checkbox | Per matched ControllerEthernet: `UploadPanelOutputs` + `UploadVirtualMatrixOutputs` + `UploadPixelOutputs` + `UploadSerialOutputs` (+ `UploadPWMOutputs` via `SetOutputs` override path). Restarts FPPD. | `FPPConnectDialog.cpp:1169-1179`, `FPP.cpp:1961-1966` |
| Playlist | combobox (player/master only) | `UploadPlaylist(playlistName)`. Playlist list pulled from `FPP::GetPlaylistItems` at discovery. | `FPPConnectDialog.cpp:1149-1151, 658-660` |
| HostName / IP / Description / Mode / Version | static text | Display only — iPad shows these in the subtitle already. | `FPPConnectDialog.cpp:218-224` |
| Upload (master) | checkbox | The Slice A toggle. Already shipped. | — |

**Bridge work for Slice B:** `uploadFseq:…` becomes one of several
methods, or the bridge gets a richer descriptor (`XLFPPUploadPlan` —
an object that carries fseqType, media policy, models choice, udp
choice, proxy/cape/playlist flags). Each per-instance config field
needs a Swift-side enum mirroring the desktop dropdown.

**Restart semantics:** any UDP-Out or Models or Cape upload sets the
restart flag; the dialog calls `Restart(true)` after the config
batch and again at end-of-uploads (`FPPConnectDialog.cpp:1197, 1432`).
`UpdateChannelRanges()` runs post-restart to refresh channel
allocations (`FPPConnectDialog.cpp:1210`). The iPad bridge must do
the same dance or uploaded configs won't activate.

### Slice C — Discovery + instance management polish (mostly P3)

- **Auth delegate ✓ shipped 2026-05-19.** New
  `XLiPadDiscoveryAuthDelegate` C++ subclass of
  `DiscoveryDelegate` lives inside `XLSequenceDocument.mm`. On HTTP
  401, the FPP class calls `PromptForPassword`, which `dispatch_async`s
  to the main queue and blocks the discovery thread on a
  `DispatchSemaphore` until the registered Swift handler fires.
  `FPPAuthPrompt` Swift enum presents a `UIAlertController` with
  username + password + "Save Password" toggle on the topmost
  view controller of the active `UIWindowScene`.
  `GetStoredPassword` / `StorePassword` use the iOS Keychain with
  `kSecClassGenericPassword`, service =
  `"xLights/Discovery/<ip>"`, account = username (typically
  "admin"), `kSecAttrAccessibleAfterFirstUnlock`. `FPPConnectSheet`
  registers the handler on appear, clears it on disappear so
  background 401s after dismissal go silent. Stored creds are
  tried automatically before prompting, so repeat discoveries on
  the same network never prompt twice.

- **Manual Add FPP** by IP/hostname — `OnAddFPPButtonClick`
  (`FPPConnectDialog.cpp:1737-1787`). Validates with
  `ip_utils::IsIPValidOrHostname`, probes via `FPP::PrepareDiscovery`
  with a single forced address, runs `AuthenticateAndUpdateVersions`
  + `probePixelControllerType` to fill the row. Demoted to P3 —
  Bonjour discovery covers the common case; users with FPPs on
  segmented networks can fall back to desktop xLights when needed.
- **Forced-IPs persistence** — `FPPConnectForcedIPs` config key,
  "|"-delimited. Pairs with manual-add. P3.
- **Re-Discover button** — `OnFPPReDiscoverClick`
  (`FPPConnectDialog.cpp:1682-1735`). Re-runs broadcast + forced,
  probes newly added. P3 — closing + reopening the sheet
  re-discovers today.
- **Host / IP sort menus** — right-click context menus on the
  HostName / IP columns; numeric IP sort uses `SplitIP`
  (`FPPConnectDialog.cpp:340-378`). P3.
- **Cape right-click menu** — Select All / Deselect All on the
  Pixel Hat/Cape column (`FPPConnectDialog.cpp:394-420`). P3.
- **Upload column right-click** — Select All / Deselect All / Sort
  by Upload / Select Subnet (`FPPConnectDialog.cpp:422-457`). P3.
- **Media column right-click** — Deselect All only
  (`FPPConnectDialog.cpp:493-514`). P3.

### Slice D — Sequence selection polish (P2)

- **Folder filter dropdown** — `ChoiceFolder` lists subdirectories of
  show folder (`GetFolderList`, `FPPConnectDialog.cpp:1799-1826`).
- **Filter dropdown** — Recursive Search vs Only Current Directory
  (`FPPConnectDialog.cpp:132-134`).
- **Date Modified column** — `DisplayDateModified`
  (`FPPConnectDialog.cpp:185-187, 1832`). iPad already has dates
  via `SequenceDatesLabel` — just needs the same format on this
  surface.
- **Channel Count column** — `DisplayPixelCount` opens the fseq
  and reads `getChannelCount()` (`FPPConnectDialog.cpp:191-193,
  1836-1847`). Useful sanity check before upload.
- **Failed-uploads retry** — `FPPConnectFailedList` config key
  stores CSV of last-run failures so the user can re-select with one
  tap (`FPPConnectDialog.cpp:745, 1065, 1463`).
- **Pre-select-from-Batch-Render-list** — `BatchRendererItemList`
  config key (`FPPConnectDialog.cpp:741-742`).

### Slice E — Proprietary vendor upload paths (P4 — out of iPad scope)

iPad targets **FPP / open-source-firmware controllers only**. The
three non-FPP code paths in desktop's `doUpload` use vendor-
specific HSEQ-style codecs for closed-firmware devices and are
deferred indefinitely:

- Falcon V4/V5 (`FPPConnectDialog.cpp:1336-1364`)
- Genius (`FPPConnectDialog.cpp:1365-1384`)
- PowerDMX (`FPPConnectDialog.cpp:1385-1404`)

iPad-side action: filter discovery results so non-FPP `fppType`
rows don't appear in the sheet (or show them disabled with a
"Use desktop xLights for this controller" hint). Revisit only if
the iPad's controller scope expands.

### Slice F — Parallel fseq generation + concurrent HTTP transfers ✓ shipped 2026-05-19

Both layers of desktop's parallelism now flow through the iPad
bridge. Multi-FPP uploads are wall-clock-bound by the slowest
target, not by N × slowest.

**Layer 1 — parallel fseq transcoding.** The new bridge method
`uploadFseq:toFPPInstances:progress:` opens the source fseq once,
calls `PrepareUploadSequence` per target, then walks the source
in FRAMES_TO_BUFFER (50) batches. Each batch is fanned out to
every target via `dispatch_apply` on the
QOS_CLASS_USER_INITIATED concurrent queue — each FPP's
`AddFrameToUpload` runs on its own thread so the per-target
transcode (zstd / sparse-encode) parallelizes cleanly. Mirrors
desktop's `parallel_for(instances, func)` at
`FPPConnectDialog.cpp:1322`.

**Layer 2 — concurrent network transfers.** Same `CurlManager::INSTANCE`
already used by the FPP class. The bridge pumps `processCurls()`
between phases (after Prepare, between every frame batch, after
Finalize) until the queue drains. Each target's bulk fseq upload
queues onto `CurlManager` inside `FinalizeUploadSequence`; with
the pump running, every target's transfer flights concurrently
over the wire. Mirrors desktop's
`while (CurlManager::INSTANCE.processCurls()) { wxYield(); }`
at `FPPConnectDialog.cpp:1259-1261`.

**Progress + cancellation.** `XLFPPUploadProgress` is now a
single multi-target protocol: `setProgress:forIPAddress:` routes
per-FPP progress into a shared `FPPUploadProgressForwarder`
(NSLock-protected per-IP dict). The runner polls `allProgress`
every 150 ms on MainActor and surfaces a per-FPP gauge row in
the upload UI so the user can see which target is dragging the
batch. Cancellation is one shared flag the bridge polls between
frame batches and on each curl pump tick.

**Runner shape.** Phase 2 changed from per-(target, sequence) to
per-sequence. For each sequence: build a target-list with
per-target media paths (gated by `FPPInstanceConfig.uploadMedia`),
call the bridge once, ingest per-target outcomes. Phase 1
(configuring) and Phase 3 (finalizing) remain per-FPP — they
need per-target restart sequencing and don't benefit from
fan-out.

### Slice G — Settings persistence parity (P3)

Per-UUID (fallback IP) config keys
(`FPPConnectDialog.cpp:1567-1597, 1622-1654`):

- `FPPConnectUpload_{uuid}` — upload toggle (Slice A has this)
- `FPPConnectUploadMedia_{uuid}` — media toggle
- `FPPConnectUploadFSEQType_{uuid}` — fseq type choice index
- `FPPConnectUploadModels_{uuid}` — 0=None, 1=All, 2=Local
- `FPPConnectUploadUDPOut_{uuid}` — 0=None, 1=All, 2=Proxied
- `FPPConnectUploadPixelOut_{uuid}` — cape toggle
- `FPPConnectUploadProxy_{uuid}` — add-proxies toggle

Global keys: `FPPConnectLocationSort`, `FPPConnectSashPosition`,
`xLightsFPPConnectSequenceSortCol` / `…SortOrder`,
`FPPConnectFilterSelection`, `FPPConnectFolderSelection`,
`FPPConnectSelectedSequences` (CSV), `FPPConnectForcedIPs` ("|"-delim),
`FPPConnectFailedList` (CSV).

### Slice H — UX strings parity (P3)

Status labels worth carrying over so the iPad's three-line hint
matches desktop:

- "Preparing Configuration" — opening
- "Checking Media and FSEQ file for {media}/{fseq}" — validation
- "Preparing FSEQ File for {fseq} ({i}/{n})" — transcode
- "Uploading {fseq} ({i}/{n})" — active upload
- "Problems Uploading" — error dialog title
- "FPP Connect Upload Complete" / "Cancelled" / "had errors or warnings" — final status

## Upload stack

| # | Item | Severity | Effort |
|---|---|---|---|
| **EX-4** | **FPPConnectDialog** — desktop's 13-column per-instance config. **Slice A (discovery + bare-bones sequence upload) shipped 2026-05-18. Slice B (per-instance Media + Cape + Add Proxies + Models + UDP Out + Playlist config) completed 2026-05-19. Slice F (parallel transcode + curl-driven concurrent transfers, per-FPP gauges) completed 2026-05-19.** Remaining: Slice C (manual Add FPP + auth + re-discover) at P3, Slice D (sequence-list polish) at P3, Slice G (settings persistence parity) + Slice H (UX strings) are polish. Slice E (proprietary vendor codecs) is P4 / out of iPad scope. | **P1** | XL (B+F done; C+D+G+H polish only) |
| EX-5 | FPPUploadProgressDialog — per-FPP gauges + cancel. **Per-FPP simultaneous gauges shipped with Slice F.** | ✓ done | — |
| EX-6 | MultiControllerUploadDialog — controller checklist + log + right-click filters; covers Falcon, WLED, PowerDMX, etc. **Tools → Bulk Controller Upload** on desktop. | P4 | M |
| EX-7 | HinksPixExportDialog — vendor-specific HSEQ format, master+2 slaves, playlists, schedule grid, USB drive export. **Tools → HinksPix Export** on desktop. | P4 | XL |
| O-12 | RemapDMXChannelsDialog — From/To/Scale/Offset/Invert grid (48 rows); .xdmxmap CSV load/save | P2 | M |
| O-15 | Print Layout button | P3 | M |

## Pixel Test

| # | Item | Severity | Effort |
|---|---|---|---|
| **O-11** | **PixelTestDialog** — 12 standard tests (Off, Chase, Chase 1/3..1/5, Alternate, Twinkle 5/10/25/50%, Shimmer, Background); per-RGB tabs; 4 selection trees (Outputs / ModelGroups / Models / Controllers) cascade checkboxes; speed/highlight/background sliders; Save/Load presets; embedded preview; ChannelTracker overlap-merge. **Tools → Test** on desktop. | P2 | XL |

Pixel Test is the field debugger that pairs with FPP Connect: after
uploading a sequence, the user wants to confirm each strand lights
up correctly before committing the show.

## Port-mapping authoring

| # | Item | Severity | Effort |
|---|---|---|---|
| O-10 | **ControllerModelDialog** drag-drop authoring — extends J-32's read-only Visualize: per-port String/DMX/Virtual Matrix, per-port protocol, per-port brightness/gamma/null pixels/colour order/group count, smart-remote A–F, auto-layout flag, bank visualisation, Print + XLSX export with smart-remote colour coding, right-click context menus, validation warnings | P4 | XL |

Also tracked alongside the strand-level wiring diagram (WV-1) in
[`future-custom-models.md`](future-custom-models.md) since both
live under "wiring authoring."

## Why deferred (apart from FPP Connect)

- Live output (lightbulb toggle) covers the most common tester
  ask — "play the sequence to my actual lights" — without needing
  any of the upload stack.
- Bulk upload / HinksPix / drag-drop port mapping are each
  multi-week sprints with no concrete user request, and they sit
  outside the FPP / WLED / ESPixelStick focus.

## When to come back

- **Now-ish:** FPP Connect (EX-4 + EX-5). External testers keep
  asking; one-tap uploads close the iPad-as-only-device story.
- After that, follow tester reports. Pixel Test (O-11) is the
  natural next pull-in once FPP Connect ships and testers want
  to verify uploads on-device.
- The P4 items (EX-6 Bulk Upload, EX-7 HinksPix, O-10 drag-drop
  port mapping) are parked indefinitely; revisit only if the
  scope of supported controllers changes.
