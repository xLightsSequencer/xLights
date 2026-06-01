# iPad Out-of-Scope Features

This is the consolidated reference for every xLights desktop feature that is
**not viable (or not worth doing) on iPad**. It is the de-duplicated companion
to "Rollup B" in `_raw-gap-analysis.md`, reorganized **by root cause** rather
than by area so the *why* is obvious at a glance.

These are distinct from *missing* features (absent but portable) and *partial*
features (present with gaps). Everything below is intentionally not planned for
parity. A handful that could plausibly flip to in-scope later are collected in
the [Could be revisited if…](#could-be-revisited-if) section at the end.

**Count: 78 distinct out-of-scope features** (the original Rollup B listed 68
after de-duplicating the "Internal automation request helper (xLightsRequest)"
row; reconciliation added 10 code-verified hard-miss items: AUTO-11 community-
script download, Pixel Test serial/DMX output, the Tip-of-the-Day feature, Wiring
+ Controller-layout Print, Check-for-Updates, Crash xLights, Log Render State,
the Generate Custom Model video wizard, and the proprietary vendor FSEQ codecs).
The single largest root cause is the **App Store no-downloaded/interpreted-code
policy** combined with **no command-line / second-process host**, which together
account for the scripting, automation, and CLI-tool families.

---

## 1. App Store policy — no downloaded or interpreted code, no external plugin dylibs

iOS App Store Review guideline **2.5.2** forbids downloading or executing code
that changes app behavior, and the iOS sandbox forbids loading executable code
(dylib/DLL) from outside the signed app bundle. This kills every embedded
interpreter, every loadable-plugin host, and every "download more scripts/
plugins" management surface.

| Feature | Desktop origin | Reason out-of-scope |
|---|---|---|
| Run Scripts (Lua / Python automation) | Tools menu script runner | The whole point is executing arbitrary downloaded community scripts — exactly what 2.5.2 prohibits. A built-in-only sandboxed Lua engine could be argued, but the user-download library makes the feature as designed out of scope. |
| Run Scripts library management (refresh / view / download) | Tools menu | Downloading additional community automation scripts to execute is the prohibited behavior; the library exists only to feed the script runner that can't ship. |
| Download community scripts from GitHub (AUTO-11) | Tools menu script library | Fetching and then executing user-community `.lua`/`.py` scripts from a remote repo is exactly the downloaded-code path 2.5.2 prohibits. The in-process JSON command dispatcher (AUTO-1, doc 12) stays in-scope and a heavily-caveated *bundled/in-app-authored* interpreted Lua engine (AUTO-7/8) is a deferred high-risk item in doc 12 — but the *download-and-run community library* surface is out of scope outright. |
| Run Scripts: View Script / Open Folder context menu | Tools menu | iOS has no "reveal in Finder" or registered external text editor for an arbitrary file; Files-app share/Quick Look is the closest idiom. |
| Python scripting engine (pybind11 / embedded CPython) | Automation / scripting | A general-purpose Python interpreter running arbitrary user `.py` files runs afoul of 2.5.2; CPython/pybind11 are also absent from the iOS deps (`lib-ios`). Desktop only compiles it when `Python.h` is present. |
| VAMP plugin timing tracks | Sequencer rows / timing | VAMP analysis relies on loadable Queen Mary plugin dylibs; loading external code is prohibited on iOS. iPad ships native on-device analyzers (onsets/tempo/chords/pitch/SNClassify) instead. |
| VAMP plugin timing-track generation | `src-core/media/xLightsVamp.h` (gated) | `xLightsVamp` is explicitly desktop-only; it loads external `.dylib`s at runtime, not permitted in the iOS sandbox. Native detectors are the replacement. |
| Download VAMP plugins prompt | Sequencer / timing | VAMP plugins are downloadable external dynamic libraries; the prompt has no purpose without a loadable-plugin host. |
| AI plugin DLL loading | AI Features | iOS forbids loading executable code from outside the signed app bundle; dynamic third-party provider plugins cannot ship. Documented in the bridge header. |

---

## 2. No raw removable-media / USB / serial / device enumeration

iOS exposes no API to enumerate or write raw removable SD/USB drives, and no
per-app selectable audio device enumeration. Workflows whose primary output is a
removable card — or whose UI is a device picker — have nothing to act on.

| Feature | Desktop origin | Reason out-of-scope |
|---|---|---|
| HinksPix Export | Tools menu | Primary output writes HSEQ/playlists/schedule to a removable SD/USB card (with a "Refresh USB Drives" picker). iOS cannot enumerate/write raw drives, and HinksPix uses proprietary firmware outside the FPP/ESPixelStick open-firmware scope the iPad bridge limits itself to. |
| HinksPix Controllers tab + multi-controller targeting | Tools menu | Foreign1/Foreign2 daughter-controller assignment and target-drive selection are intrinsic to the removable-media + proprietary-firmware workflow iOS cannot serve. |
| HinksPix Playlists tab (build/order/save playlists) | Tools menu | Only meaningful as input to the `.ply` card/controller output, which is desktop-only. No standalone value on iPad. |
| HinksPix Schedule tab | Tools menu | The day/time schedule grid is written to the controller/card schedule, dependent on the desktop-only proprietary-controller + removable-media path. |
| HinksPix Export to SD Card vs Upload (Refresh USB Drives) | Tools menu | iOS exposes no API to enumerate or write raw removable SD/USB drives, so the primary HinksPix output channel cannot be implemented. |
| Audio output device selection | Media & Audio | iOS does not expose per-app selectable audio output devices the way SDL/AVAudioEngine enumeration does on desktop; routing is OS-controlled (AVAudioSession / Control Center / AirPlay). A device picker would have nothing to enumerate. |
| Pixel Test: serial / USB-DMX test output | `PixelTestDialog.cpp` (`ControllerSerial` / serial-port test items) | The dialog drives test patterns directly out of host serial / USB-DMX ports (`ControllerSerial::GetPort`, per-`Serial Port N` test items). iOS has no host serial driver, so the serial/DMX half of Pixel Test cannot run. The serial controller *record* stays editable and FPP-proxy-uploadable (doc 07), but the device is never the live serial driver. **Note:** the *network* test-pattern half (sACN/ArtNet/DDP) is feasible and tracked as the deferred CTL-20 in doc 07 — it is XL effort with no shared-core test engine, so deferred rather than out-of-scope. |

---

## 3. No bundled FFmpeg encode (video export / transcode)

The iPad library does not build the FFmpeg encode path; the bridge explicitly
documents "iPad cannot transcode video — desktop-only." Any feature that encodes
or muxes video, or configures codec/bitrate, has no pipeline behind it. (Decode
goes through VideoToolbox automatically; only *encode* is missing.)

**Why not just bundle the whole FFmpeg library on iPad?** Three compounding
reasons: (1) **size** — a full static FFmpeg with the encoders/muxers xLights
relies on adds roughly **~50 MB** to the signed binary, a large cost for a
single non-core feature; (2) **licensing** — the encoders xLights uses (x264 et
al.) pull GPL/commercial-licensed components, which is a meaningful obligation to
take on for App Store distribution; (3) **App Store review risk** — large
vendored media stacks invite extra scrutiny and rejection risk. The native
substitute, **AVAssetWriter + VideoToolbox encode**, covers **MP4/MOV (H.264 /
H.265) only** — it has **no GIF encoder** and no arbitrary-container muxing, so
even the in-scope-later path (see *Could be revisited if…*) is a partial
replacement, not a drop-in. Note that *decode* via VideoToolbox already works for
playback/import, so only the encode/transcode side is affected.

| Feature | Desktop origin | Reason out-of-scope |
|---|---|---|
| Export House Preview Video | `src-ui-wx/media/VideoExporter.cpp` | Desktop encoder is wx-only and uses FFmpeg encode. An AVAssetWriter exporter fed by the Metal house preview is net-new XL work with no shared-core reuse. |
| Export rendered sequence as video | `src-ui-wx/media/VideoExporter` (GenericVideoExporter) | wx-UI desktop component relying on FFmpeg encode the iPad lib does not build. Would need an AVAssetWriter reimplementation; not a sequencing parity blocker. |
| Incompatible-video conversion on import | VideoTranscoder / MediaCompatibility | Transcode path is desktop-only; no FFmpeg encode in the iPad lib. iPad should warn-and-skip rather than transcode in place. |
| Command: exportVideoPreview | Automation / REST | Desktop encodes the preview to MP4/H.264 via FFmpeg. iPad does not bundle the encode path; on-device H.264 would need a separate AVFoundation/VideoToolbox encoder pipeline. |
| Video Export Codec (pref) | Preferences | No video-export pipeline on iPad, so a codec choice has nothing to configure. |
| Video Export Bitrate (pref) | Preferences | Same — no export pipeline, so there is no bitrate to set. |

---

## 4. wxAUI desktop windowing has no iPad analogue

A "perspective" is a wxAUI `SavePerspective()` geometry string describing the
dock/float/tab/size of ~17 panes. iPad has no AUI manager — it uses a fixed
SwiftUI split-pane layout with Preview/Inspector toggles and detach-to-scene,
all persisted automatically via `@AppStorage`. There is no arbitrary pane
geometry to capture, manage, or restore.

| Feature | Desktop origin | Reason out-of-scope |
|---|---|---|
| Perspective layout engine (wxAUI dock/float/persist) | View / Windows | wxAUI free-form docking with serialized perspective strings is a wx-desktop construct; iPadOS windowing is scene-based (Stage Manager / Split View). Re-implemented with native idioms, not ported. |
| Perspectives: Save Current | View / Windows | No AUI pane geometry to capture; iPad's pane widths/heights and detached-scene state are already auto-persisted. |
| Perspectives: Save As New | View / Windows | No wxAUI layout to snapshot under a name; named presets would be net-new design, not a port. |
| Perspectives: Edit/Load (manager panel) | View / Windows | Operates on AUI perspective strings in rgbeffects; with no AUI docking there is nothing to manage. |
| Perspectives: Auto Save (per-machine) | View / Windows | iPad's analogous live-layout persistence is already always-on and native, so a toggle is unnecessary and inapplicable. |
| Perspectives: dynamic per-perspective menu list / quick switch | View / Windows | Depends entirely on the saved-perspectives list, which doesn't exist on iPad. |
| Windows: Perspectives shortcut item | View / Windows | Duplicate entry point to the manager, itself out-of-scope. |
| Suppress Dock: House Preview | View / Windows | Forces a wxAUI pane to stay floating; iPad has no dock-vs-float-vs-suppress state machine. Detach-to-scene is the equivalent. |
| Suppress Dock: Model Preview | View / Windows | Same — no AUI docking model, so no floating-only state to enforce. |
| Reset Toolbars | View / Windows | No wxAUI toolbars to reset; the iPad toolbar set is a fixed SwiftUI layout. |
| Double-click to maximize preview pane | Layout: Preview | No resizable in-window AUI pane to maximize/restore; windowing is OS-managed. Double-tap is repurposed for camera reset. |
| Effect Assist Window mode (pref) | Preferences | Desktop Effect Assist is a docked AUI pane; iPad uses inline inspector rows (XYCenterPadView, MorphLineEditorRowView), so an On/Off/Auto mode has nothing to govern. |

---

## 5. No command line / argv / shell / second process

iOS apps have no command-line argv, no shell, cannot spawn a second app
instance, cannot launch another app's binary, and cannot self-terminate. The
entire CLI tool, headless, and launch-flag family is inapplicable. (Verified: no
`cmdLine`/`argc`/`argv` handling in `src-iPad`.)

| Feature | Desktop origin | Reason out-of-scope |
|---|---|---|
| xlDo command-line tool | `xlDo/` (separate CLI target) | Standalone CLI binary that talks to a running xLights over its localhost automation port. iOS has no shell, no process spawning, and no automation port. |
| xlDo: template files with `%P1..%P9` parameter substitution | xlDo | No CLI, no template-invocation surface, no automation port. |
| xlDo: `-s` output script generation (set/EXPORT vars) | xlDo | Writes JSON values into a shell/batch script for a calling shell; there is no shell on iOS. |
| xlDo: startxLights launcher command | xlDo | Launches the xLights binary on a chosen port and polls until it responds; iOS does not allow one app to exec/launch another. |
| App launch: headless render (`-r`) and check-sequence (`-cs`) | App startup | No argv; cannot run headless/exit-on-completion (self-exit is forbidden). |
| App launch: `-a`/`-b` port, `-o` output-on, `-s`/`-m` dirs, `-w` wipe | App startup | All command-line switches; iOS provides no argv. Show/media dirs, output toggle, and reset are reachable through the iPad UI instead. |
| Automation port preference (Disabled / Port A / Port B) | Preferences / automation | Exists only to configure the A/B HTTP listener and `-a`/`-b` flags so two desktop instances + xFade/xSchedule coexist on one machine — not an iPad scenario. |
| Internal automation request helper (xLightsRequest) | `wxUtilities.cpp` | A wx helper that builds a URL to the local automation port for self/companion-app HTTP calls. iPad has no automation port and drives the core directly via the in-process `XLSequenceDocument` bridge, making an HTTP self-call pointless. |
| Command: closexLights | Automation / REST | iOS apps cannot programmatically terminate themselves (calling `exit()` is rejected; the OS owns app lifecycle). No equivalent of posting `wxID_EXIT`. |
| Open New xLights Instance | File & Sequence Lifecycle | iOS apps are single-process; you cannot spawn a second independent instance. Multi-scene windows share one process/app state. |
| fseq_convert standalone tool | Render engine | iOS has no user-facing CLI; a standalone binary cannot be shipped or invoked in the App Store sandbox. (The underlying FSEQFile conversion code is shared and could be exposed as an in-app action if needed.) |

---

## 6. Desktop/Windows-only or companion-app IPC

Features that depend on a Windows-only backend, a desktop companion app (xFade /
xSchedule) reached over OS shared-memory IPC, or desktop-only hardware drivers.
The companion apps don't run on iPadOS and there is no shared-memory IPC channel
to another local app sandbox.

| Feature | Desktop origin | Reason out-of-scope |
|---|---|---|
| xFade/xSchedule sync port | Preferences | xFade/xSchedule integrate via OS shared-memory ports (A/B); they don't run on iPadOS and there is no shared-memory IPC to another sandbox. |
| Jukebox tooltip/state export for xSchedule | jukeboxPanel (`XLIGHTS_JUKEBOX` label IPC) | Exists solely so the desktop xSchedule app can read the panel via wx label introspection. xSchedule moved to its own repo and has no iPad counterpart, so this IPC contract has no consumer. |
| Command: playJukebox | Automation / REST | The Jukebox is a desktop panel for storing/triggering saved effects; no Jukebox panel exists on iPad, so the play-by-index command has nothing to drive. |
| Command: jukeboxButtonTooltips / jukeboxButtonEffectPresent | Automation / REST | Depend on the desktop Jukebox panel which does not exist on iPad; no data source. |
| SpaceMouse / 3Dconnexion 6-DOF navigation | Layout: Preview | iPadOS has no 3Dconnexion HID driver or public 6-DOF input API; the SDK is desktop-only. Multi-touch + Pencil is the native equivalent. |
| Suppress Dark Mode (Windows only) | Preferences | Windows-only override (already hidden on macOS/Linux). iPadOS appearance follows the system setting; an override would conflict with platform HIG. |
| Hardware Video Renderer (Windows) | Preferences | Windows-only DirectX11/CUDA/QSV/Vulkan/AMF decode-backend choice (hidden on macOS/Linux). None of these exist on iPadOS, which uses VideoToolbox. |
| Hardware Video Decoding (FFmpeg backend switch) | Preferences | On Apple platforms video decode goes through VideoToolbox (hardware) automatically; the desktop toggle picks between FFmpeg backends iPad doesn't use. |

---

## 7. A platform-native idiom already replaces it

Not a gap — the iPad simply uses the OS-standard affordance, which makes the
desktop control redundant or its toggle meaningless. Building the desktop
version would fight the platform and add no user value.

| Feature | Desktop origin | Reason out-of-scope |
|---|---|---|
| Custom HSV/RGB color picker dialog | Color & palettes | iOS provides a rich native picker (`UIColorPickerViewController` via SwiftUI `ColorPicker`); a custom HSV-disc dialog would fight the OS. |
| Use custom color picker (experimental) (pref) | Color & palettes | The desktop toggle switches between the OS picker and the wx custom picker; iPad uses SwiftUI `ColorPicker` throughout, so there is no alternative to switch to. |
| Reset panel when changing effects (pref) | Color & palettes | A wxConfig-backed pref for the persistent docked ColorPanel; the iPad inspector rebuilds per-selection and has no equivalent panel/config to reset. |
| Palette display size preference (Normal/Large) | Color & palettes | Manages limited width in the desktop's fixed ColorPanel; the iPad inspector lays palette rows full-width with dynamic type, so a size toggle has no purpose. |
| Revert To… versioned backups (macOS) | File & Sequence Lifecycle | Uses macOS `NSFileVersion` document-revision APIs; iOS provides no on-disk versioned-revision browser for arbitrary sandboxed files. |
| Zoom To Cursor (pref) | Preferences | No mouse cursor on iPad; pinch-zoom always centers on the gesture centroid (the touch equivalent), unconditionally. A toggle would have no alternative. |
| Auto Show House Preview (pref) | Preferences | iPad has no floating/auto-appearing preview window; the house preview is an embedded pane or an explicitly-opened detached scene, governed by iPadOS scene lifecycle. |
| GPU Rendering (pref) | Preferences | The iPad render context is already configured for the device's capabilities; there is no meaningful CPU-vs-GPU toggle, and forcing CPU would only hurt. |
| Low Definition Render (pref) | `iPadRenderContext::IsLowDefinitionRender()` | Hard-coded `true` as a deliberate memory/perf constraint for 4 GB devices; a user toggle would risk OOM on large models. |
| Shaders on Background Threads (pref) | Preferences | Already hidden on macOS/Linux; iPad runs ShaderEffect single-threaded on Metal via ANGLE by design, so background-thread shader rendering doesn't apply. |
| Effect status / hover tooltip | Sequencer grid | Touch UI has no hover/cursor state or status bar; hover tooltips and cursor-zone feedback don't translate to a finger-driven grid. |
| Bulk upload ESC cancel | Controllers: upload | No physical ESC key in the touch idiom; replace with an on-screen Cancel button when the bulk dialog is built. |
| Custom Model Method Picker (macOS) | `src-ui-wx/mac/CustomModelMethodPickerDialog.cpp` | A macOS-AppKit helper for choosing the custom-model creation method; iPad's Add-Model flow (AddModelSheet + Map-from-Lights) is its own native entry point, so there is nothing to port. |
| Convert log / status reporting | ConvertLogDialog | The status surface for the Convert dialog, itself deferred/missing on iPad. iPad already has a live spdlog View Log sheet and import-error alerts; only meaningful once/if the Convert dialog is ported. |
| Crash xLights (intentional-crash telemetry tool) | `xLightsMain.cpp:1137` (`MenuItem_CrashXLights` / `ID_MNU_CRASH`) | A developer diagnostic that deliberately crashes the app to exercise the crash-report uploader. On iPad this duplicates the already-shipped Package Logs + crash telemetry (`XLDiagnosticUploader`); a deliberate-crash menu item adds no user value and would look hostile in an App Store build. |
| Log Render State (render-state dump) | `xLightsMain.cpp:1139` (`MenuItem_LogRenderState` / `ID_MNU_DUMPRENDERSTATE`) | A developer diagnostic that dumps the render thread/job state to the log. iPad already has a live in-app View Log sheet plus Package Logs, which cover the same debugging need; the explicit dump menu item is redundant. |
| Disable key acceleration when held down (pref) | Preferences | Governs keyboard key-repeat acceleration; iPad is touch-first and hardware-keyboard repeat is managed by iPadOS. Marginal even on desktop. |
| Tip Of The Day feature | `TipOfTheDayDialog.cpp` | The desktop onboarding "tip of the day" modal (curated tips with difficulty levels + recycle) is a low-value desktop-onboarding idiom; iPadOS onboarding leans on contextual hints and the App Store description rather than a startup tips modal. The two related prefs below are downstream of this. |
| Tip Of The Day minimum level (pref) | Preferences | No Tip Of The Day feature on iPad; a minimum-difficulty choice has nothing to govern. |
| Tip Of The Day recycle tips (pref) | Preferences | No tips feature on iPad, so there are no seen-tips to recycle. |
| Wiring-diagram Print (WV-6) | `WiringDialog.cpp:751-763` (`ID_MNU_PRINT` → `wxPrinter` + `WiringPrintout`) | Per-model wiring diagram print routes through `wxPrinter`/`wxPrintData`, a desktop print-dialog construct. Printing is a weak iPad need; the native idiom is to render the wiring diagram to a PNG and hand it to `UIActivityViewController` → Save / Share / AirPrint. Build the *share-PNG* path if/when the wiring diagram itself is ported (it lives in `future-custom-models.md`). |
| Controller-layout / model-preview Print (O-15) | `LayoutPanel.cpp:10088-10167` (`Printout : wxPrintout` over `ModelPreview` + `wxPrinter`) | The layout/preview print path is a `wxPrintout` subclass driven by `wxPrinter`/`wxPrintDialogData`. iPad has no `wxPrinter`; route a captured layout/preview image through `UIActivityViewController` → AirPrint instead of porting the print dialog. |
| Check for Updates / UpdaterDialog | `UpdaterDialog` (Help menu) | App distribution and updates are handled by the App Store (and TestFlight for pre-release). There is no in-app self-update flow to build; the desktop version-check + downloader has no iPad analogue. |

---

## 8. Heavy desktop-only pipeline or deliberate proprietary scope gate

Not a hard platform limit and not an App Store policy block — these are
desktop-only (or deferred) because the underlying pipeline is large,
wx/OpenCV-bound, or proprietary, and the parity cost vastly outweighs the value
today (often with a native partial substitute already shipping).

> **Proprietary controllers are DEFERRED, not permanently out-of-scope.** For
> now the iPad app limits controller **upload / FPP Connect / Visualize** to
> open-source firmware — **FPP, WLED, ESPixelStick** (the three
> `.xcontroller` configs declaring `<OpenSourceFirmware>`). This is a
> scope + monetization decision, *not* a platform block (iOS HTTP-uploads to any
> vendor; the handlers compile from core). Proprietary controllers **may** ship
> later, **low priority, most likely behind an in-app purchase / subscription
> tier** — see [`07-controllers-and-upload.md`](07-controllers-and-upload.md)
> "Open-source-firmware scope". The genuinely platform-blocked HinksPix SD-card
> authoring + raw serial/DMX output (§2) stay hard out-of-scope regardless.

| Feature | Desktop origin | Reason out-of-scope / deferred |
|---|---|---|
| **Proprietary / closed-firmware controller support (upload, FPP-Connect codecs, HTTP-scan discovery, Visualize)** — Falcon V4/V5, Genius, PowerDMX, Pixlite/Advatek, SanDevices, J1Sys, HinksPix-network, … | `07-controllers-and-upload.md` "Open-source-firmware scope"; `caps.openSourceFirmware` gate (`XLSequenceDocument.mm:11779`) | **Deferred, likely future IAP/subscription tier — not a platform limit.** The iPad intentionally gates these surfaces to the open-source set (FPP/WLED/ESPixelStick) today; the HTTP upload plumbing works on iOS and every vendor handler already compiles from `src-core/`, so this is a product/monetization choice, not impossibility. Filtered/disabled now with a "Use desktop xLights" hint. |
| Generate Custom Model wizard (video bulb-detection) (MA-2 / T-11) | `GenerateCustomModelDialog.cpp` (`PAGE_CHOOSEVIDEO` … `PAGE_BULBIDENTIFY`; OpenCV-style frame `Threshold`/`RemoveBackground`/`SetFrameDelta` pipeline) | The desktop wizard records a video of the lit prop and runs a multi-pass OpenCV-style image pipeline (grayscale → threshold → erode/dilate → background-subtract → frame-delta) to locate each bulb and build the custom-model grid. The whole pipeline is wx/desktop-bound with no shared-core reuse, and OpenCV is not in the iPad deps. **Partial substitute (deferred, not impossible):** iPad's ARKit-based *Map-from-Lights* flow already does on-device spatial light detection, which covers the same user goal by a different route. Porting the recorded-video pipeline itself is deferred. |
| Proprietary vendor FSEQ codecs (Falcon V4 / V5, Genius, PowerDMX) | `07-controllers-and-upload.md`; `FSEQFile.h` (open codecs are `none` / `zstd` / `zlib`) | iPad's FSEQ writer ships only the open compression types (`none`, `zstd`, `zlib`) and the upload surface is gated to open-source firmware (**FPP / WLED / ESPixelStick**) — see doc 07. The vendor-specific FSEQ encodings (Falcon V4/V5, Genius, PowerDMX) ride proprietary firmware/codecs outside that open-firmware scope; they sit in the same deferred-(future-IAP) proprietary tier as the row above. Naming them makes the proprietary-codec exclusion explicit, since prior plans documented the firmware gate but not the codec gate. |

---

## Could be revisited if…

A small number of items are out-of-scope *today* but could become in-scope if a
specific capability is built or a device constraint lifts:

- **Video export via AVAssetWriter** — *Export House Preview Video*, *Export
  rendered sequence as video*, *Command: exportVideoPreview*, and the *Video
  Export Codec / Bitrate* preferences all become in-scope together if an
  AVAssetWriter-based H.264/H.265 encoder fed by the Metal canvas is built. This
  is net-new work with no shared-core reuse, not a wiring task.
- **Full-definition render on Pro devices** — *Low Definition Render* and *GPU
  Rendering* are hard-locked for the 4 GB memory floor today. If memory headroom
  on Pro-class iPads allows full-def GPU compute, these could become user
  preferences.
- **Named iPad layout presets** — the *Perspectives* family could be
  re-conceived as native named layout presets (which panes detached, sidebar
  widths). That is net-new SwiftUI layout-state design, not a port of the wxAUI
  engine.
- **Proprietary-controller support behind an IAP / subscription tier** — upload,
  FPP-Connect codecs, HTTP-scan discovery, and Visualize for closed-firmware
  vendors (Falcon, Genius, PowerDMX, Pixlite/Advatek, SanDevices, J1Sys,
  HinksPix-network, …) are gated out today by product choice, not platform
  limit. They become in-scope if/when an in-app purchase or subscription tier is
  built to unlock them (low priority). The HTTP plumbing and vendor handlers
  already exist in `src-core/`; the work is the entitlement/paywall gating plus
  surfacing the already-filtered affordances. (The vendor FSEQ codecs are extra
  reimplementation work on top.) HinksPix SD-card authoring + raw serial/DMX
  output stay hard out-of-scope even then.
- **In-app FSEQ conversion action** — the *fseq_convert* CLI tool can't ship,
  but its conversion code is shared `src-core`, so the same functionality could
  be surfaced as an in-app action if ever needed.
- **Built-in-only Lua scripting** — the community-*download* runner stays out
  (AUTO-11, §1). A sandboxed interpreted Lua engine restricted to **bundled or
  in-app-authored** scripts (never downloaded) is tracked as a deferred,
  heavily-caveated high-risk item (AUTO-7/8) in
  [`12-ai-and-automation.md`](12-ai-and-automation.md), while the in-process JSON
  command dispatcher (AUTO-1) is fully in-scope there. The old "JIT is App-Store-
  hostile" objection is moot — iPad Lua would be *interpreted*, not JIT — but the
  user-script-API mapping cost and App Store review risk remain, which is why it
  stays deferred rather than scheduled.

---

## Reclassified — NOT out-of-scope (do not re-add here)

These appeared as candidate out-of-scope items in earlier plans/datasets but were
resolved as **in-scope or portable** during reconciliation; recorded so they are
not re-added to this doc.

- **PR #6311 "ctrls tab to Layout" (select models by controller / port)** — the
  opaque "controller-source-tree" entry deferred in
  `phase-j-layout-editor.md:150` is this *closed* desktop PR: a Layout-panel tab
  that groups/selects models by their assigned controller and port (incl. shadow
  models). It is a portable model-selection/navigation aid with full shared-core
  backing (`OutputManager` / `UDController` port maps), **not** a platform block.
  It belongs in the layout/controllers backlog if pursued, not in this
  out-of-scope doc.
- **Custom KeyBindings editor** — in-scope as a low-priority **PREF-29** in
  [`11-preferences-settings.md`](11-preferences-settings.md): a remap UI over the
  shared `KeyBindings` core. iPad supports hardware keyboards + SwiftUI keyboard
  shortcuts, so the prior README "hard miss" framing was conservative and is
  superseded — deliberately **not** listed here.
