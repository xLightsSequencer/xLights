# 07 · Setup, Controllers, Outputs & Upload

> The iPad ships a full **Controllers tab** in the Layout Editor with add
> (Ethernet/Serial/Null), delete, reorder, activate/inactivate,
> unlink-from-base, search/filter, a deep **editable property pane**
> (name/desc/id/active/auto-layout/auto-upload/auto-size/full-control/
> brightness/gamma/suppress/monitor + vendor/model/variant cascade +
> Ethernet IP/multicast/proxy/protocol/priority + Serial port/protocol/
> speed/i2c/spi/channels + caps "extra" props), **network discovery**
> with a dedicated mismatch-resolution sheet, a fully **editable
> Visualize/wiring sheet** (port-protocol picker, smart-remote, model
> controller properties, set-start-channel, drag-drop assign / move-to-
> port / unassign, CSV+JSON wiring export), **single-controller
> input+output upload** gated to open-source firmware, a **Vendor
> catalog browser**, **Map-from-Lights** (FPP structured-light scan),
> and a full **FPP Connect** sheet (discover, per-FPP config, parallel
> fseq fan-out). The theme-07 controllers cluster is now at parity:
> **bulk multi-controller upload**, **global output settings**
> (Controller Sync / E1.31 Sync Universe / Global FPP Proxy / Global
> Force-Local-IP / Max-Suppress-Frames), **per-universe Output editing**
> (Universe #, Universes count, UniversePerString, IndivSizes,
> per-universe channels), **per-controller Force-Local-IP**, the
> **"Open Proxy" browser** shortcut, **ping/health LED**,
> **FPP-proxy pre-upload validation**, and **auto-upload on
> output-enable** all ship on iPad. The **global "Export Controller
> Connections" XLSX**, the **controller sort menu**, **FPP Connect
> immediate-output upload for non-FPP discovered devices**, and the
> **FPP Connect "Upload Controller" input-universe push (#2747)** now
> ship on iPad too. The genuine remaining iPad gap is **pixel test**
> (infeasible — raw DMX/sACN/ArtNet output is sandbox-blocked).
> Closed-firmware vendor uploads (Falcon, SanDevices, HinksPix, AlphaPix,
> J1Sys, Minleon, vendor-FW Pixlite) are **restricted/IAP-gated P3** on
> iPad.

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| Add Ethernet controller | toolbar/menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ButtonAddControllerEthernet`; iPad tab "+" menu → `addController(ofType:"Ethernet")`. |
| Add Serial controller | toolbar/menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ButtonAddControllerSerial`; iPad "+" menu. iPad has no hw serial but allows editing a desktop-defined serial controller. |
| Add Null controller | toolbar/menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `ButtonAddControllerNull`; iPad "+" menu. |
| Delete controller | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop context menu / `ButtonControllerDelete`; iPad swipe + context-menu + confirm alert. |
| Reorder controllers | gesture | ✅ | ✅ | parity | P2 | easy | feasible | Desktop drag + Up/Down bitmap buttons; iPad `onMove`→`moveController(toIndex:)`. iPad disables reorder while filtered. |
| Rename controller | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop property grid "Name"; iPad detail-pane editable, re-anchors selection. |
| Controller description | dialog/panel | ✅ | ✅ | parity | P3 | easy | feasible | Both expose editable Description (iPad `CtrlStringProp(@"Description")`). |
| Controller Id field | dialog/panel | ✅ | ✅ | parity | P3 | easy | feasible | Both editable Id (0..65535). |
| Active / xLights-Only / Inactive | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop context menu IDs; iPad context-menu state buttons + Active enum in detail pane. FromBase gate matched. |
| Auto Layout / Auto Upload / Auto Size | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad bool props mirror desktop adapter flags. |
| AutoSize uncommon universe warning (#4123) | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | Desktop warns when AutoSize enabled on E1.31 with universe size ≠ 170/510/512 (`ControllerPropertyAdapter.cpp:327`). iPad: `controllerAutoSizeUniverseWarning(name:)` bridge method (`XLSequenceDocument.mm`) + `AutoSizeUniverseWarningModifier` alert in `LayoutEditorView.swift`. |
| FPP Player-Only model → default Active to xLights Only (#6453) | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | Desktop: `ControllerPropertyAdapter::HandlePropertyEvent("Model")` checks `IsPlayerOnly()` and sets Active to "xLights Only" (`ControllerPropertyAdapter.cpp:395`). iPad: same check added to the "Model" branch of `setControllerProperty` in `XLSequenceDocument.mm:12866`. |
| Full xLights Control + default brightness/gamma | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `FullxLightsControl` / `DefaultBrightnessUnderFullxLightsControl` / `DefaultGammaUnderFullxLightsControl`. |
| Suppress duplicate frames | dialog/panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad `SuppressDuplicates` bool. |
| Monitor | dialog/panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad `Monitor` bool. |
| Vendor / Model / Variant cascade | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | Shared `ControllerCaps::GetVendors/Models/Variants`; iPad enum cascade. |
| Caps "extra" properties | dialog/panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad `ControllerExtra.*` enum/string from `GetExtraPropertyDefs()`. |
| Last Input/Output Upload timestamp | dialog/panel | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop shows two read-only fields at the bottom of the property pane (`Controllers_PropertyEditor` in `src-ui-wx/app-shell/TabSetup.cpp`). Persisted via `GetXLightsConfig()` (JSON app settings, scoped by show directory via `MakeControllerTimestampKey` in `src-core/utils/UtilFunctions.h`) — no `NetworkChange()` call, so uploading never marks the show dirty. Recorded from both manual upload (`UploadInputToController`/`UploadOutputToController` at `src-ui-wx/app-shell/TabSetup.cpp`) and FPP Connect (`src-ui-wx/controllers/FPPConnectDialog.cpp`). `FormatTimestamp()` centralized in `src-core/utils/UtilFunctions.h`. iPad controller detail pane has no equivalent. |
| Controller IP (Ethernet) | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | Both editable; iPad shows in row secondary line. |
| Multicast toggle (Ethernet) | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `Multicast` bool; locks IP field. |
| Ethernet output protocol (E1.31/ArtNet/DDP/ZCPP/…) | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad `Protocol` enum from caps. |
| Priority (Ethernet) | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `Priority` int 0..100. |
| Managed flag (Ethernet) | dialog/panel | ✅ | ✅ | parity | P3 | easy | feasible | Read-only on both. |
| Per-controller FPP Proxy IP | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `FPPProxy` string (Ethernet + FPP-serial). |
| Force Local IP (per-controller) | dialog/panel | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `ForceLocalIP` enum; iPad `ForceLocalIP` enum descriptor in `controllerPropertiesForName` (`src-iPad/Bridge/XLSequenceDocument.mm`) + setter, options from `ip_utils::GetLocalIPs()`. |
| Universe number / Universes count | dialog/panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad ethernet "Output" section: `Universe`/`Universes`/`UniversePerString`/`IndivSizes`/uniform `Channels`/per-universe `Channels/<n>` descriptors + setters in `controllerPropertiesForName`/`setControllerProperty` (`src-iPad/Bridge/XLSequenceDocument.mm`), shown for E1.31/ArtNet/KiNET. |
| Serial port selection | dialog/panel | ✅ | 🟡 | parity | P1 | easy | feasible | Desktop port dropdown; iPad enumerates system ports or freeform (no hw serial on iPad) + FPP `tty/i2c/spi` ports. |
| Serial output protocol (DMX/LOR/Renard/Minleon/…) | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad `SerialProtocolOptions`. |
| Serial baud / speed | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad `SerialBaudOptions`, grey-out when protocol fixes baud. |
| Serial i2c / spi / generic prefix-postfix | dialog/panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad covers i2c device, SPI kHz, generic-serial prefix/postfix. |
| Serial channels (auto-size aware) | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `Channels` int, read-only when AutoSize. |
| Null controller channels | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `Channels` 1..1000000. |
| Models-on-controller list / channel range | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop "Models" prop + range; iPad detail-pane member list, tap to select. |
| Discovery (auto-detect controllers) | toolbar/menu | ✅ | ✅ | parity | P1 | medium | feasible | Desktop `ButtonDiscover` thread sweep; iPad "+"→Discover detached task → `runControllerDiscovery`. |
| WLED mDNS auto-discovery (`_wled._tcp` → DDP, auto-size/auto-layout) | toolbar/menu | ✅ | ✅ | parity | P1 | easy | feasible | Core `WLED::PrepareDiscovery` (`src-core/controllers/WLED.cpp:145`) → `Discovery::AddBonjour("_wled._tcp", …)`. Desktop wires it in `PrepareAllControllerDiscovery` (`DiscoveryHelpers.cpp:76`); iPad now calls `WLED::PrepareDiscovery(discovery)` in `runControllerDiscovery` (`XLSequenceDocument.mm:13496`) and declares `_wled._tcp` in `NSBonjourServices` (`Info.plist`) so iOS allows the browse. |
| Discovery mismatch resolution (IP/name) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop inline `wxMessageDialog` prompts (TabSetup ~1569/1612); iPad dedicated `DiscoveryMismatchModifier` sheet → `applyDiscoveryMismatch`. |
| Discovery auth (401 credentials) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop `DiscoveryAuthDialog`; iPad `setFPPAuthPromptHandler` + `FPPAuthPrompt`. |
| Controller filter / search | tab | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad name/vendor/model/IP search; desktop has no search box (small list). |
| Controller list sort (by name/id/ip/proxy/vendor/protocol) | context-menu | ✅ | ✅ | parity | P3 | medium | feasible | Desktop "Sort" submenu (`TabSetup.cpp:2504`). iPad Controllers-tab header menu → Sort submenu (`LayoutEditorView.swift`), bridged through `sortControllers(byMode:)` → `OutputManager::SortControllersby*` (same core reorder the desktop calls; persists + marks dirty). |
| Controller list column sort (header click) | gesture | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `OnListControllersColClick`; iPad list isn't column-based. |
| Open browser to controller | context-menu/gesture | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `ButtonOpen` / double-click; iPad double-tap row + context-menu "Open <ip>" + detail button → `UIApplication.shared.open`. |
| Open FPP Proxy in browser | toolbar | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `Button_OpenProxy`; iPad detail-pane "Open Proxy" button (`LayoutEditorControllerDetailView`) opens `http://<proxy>/` when the controller has an FPP proxy set. |
| Ping / health LED | toolbar | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `LedPing` colour-codes last ping. iPad `pingController` bridge wraps the shared core `Controller::Ping()` (HTTP reachability probe on non-Windows hosts) + colour-coded status dot on controller rows / detail pane with tap-to-refresh. |
| Save setup | toolbar | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ButtonSaveSetup` (turns red when dirty); iPad folds into unified Save via `SaveLayoutChanges` + `hasUnsavedLayoutChanges`. |
| Unlink from Base Show Folder | context-menu | ✅ | ✅ | parity | P2 | easy | feasible | Both gate when controller is FromBase; iPad `unlinkControllerFromBase`. |
| Base-show link badge | tab | 🟡 | ✅ | desktop-missing | P3 | easy | feasible | iPad blue link icon; desktop shows FromBase via different styling, no dedicated badge. |
| Open-source-firmware badge | tab | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad green shield when `caps.openSourceFirmware`; desktop has no indicator. |
| Visualize / wiring view | dialog/sheet | ✅ | ✅ | parity | P1 | medium | feasible | Desktop `ControllerModelDialog`; iPad `ControllerVisualizeView` (`wiringForController`). Both editable. |
| Port protocol selection (visualize) | dialog | ✅ | ✅ | parity | P1 | medium | feasible | iPad confirmation-dialog picker → `setPortProtocol`. |
| Smart-remote assignment (visualize) | dialog | ✅ | ✅ | parity | P1 | hard | feasible | iPad `ModelSmartRemoteSheet` + `smartRemoteCapabilitiesForController`. |
| Model→port assignment / chaining (visualize) | dialog/gesture | ✅ | ✅ | parity | P1 | hard | feasible | iPad Transferable drag-drop + `assignModel(...afterModel:smartRemote:)`, chain-aware. |
| Move model to port (visualize) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | iPad `MoveToPortSheet` + `portCountsForController`. |
| Remove model from port (visualize) | dialog/gesture | ✅ | ✅ | parity | P2 | medium | feasible | iPad `removeModelFromController`. |
| Set start channel (visualize) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | iPad `SetStartChannelSheet`. |
| Model controller-properties (brightness/gamma/color-order/nulls/groups) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | iPad `ModelControllerPropertiesSheet` + `controllerConnectionForModel`. |
| Wiring export (per controller) — CSV | dialog | ✅ | ✅ | parity | P3 | easy | feasible | iPad Visualize "Export as CSV" → `exportWiringCSV`. |
| Wiring export (per controller) — JSON | dialog | ✅ | ✅ | parity | P3 | easy | feasible | iPad Visualize "Export as JSON" → `exportWiringJSON`. |
| Export Controller Connections (global XLSX) | menu | ✅ | ✅ | parity | P3 | medium | feasible | Desktop `OnMenuItem_ExportControllerConnectionsSelected` → libxlsxwriter all-controllers workbook. iPad Controllers-tab header menu → "Export Connections…" → `exportControllerConnections(toPath:)` (same core `UDController::ExportAsCSV` + libxlsxwriter, smart-remote shading) → `.fileExporter`. iPad exports the full field set rather than prompting (desktop shows a field picker). |
| Upload output (wiring) | toolbar/menu | ✅ | ✅ | parity | P1 | hard | restricted | iPad `uploadOutputForController`, gated to open-source firmware (osf). Closed FW = restricted P3. |
| Upload input (universes) | toolbar/menu | ✅ | ✅ | parity | P1 | hard | restricted | iPad `uploadInputForController`, osf-gated. |
| Linked input+output upload | toolbar | ✅ | ✅ | parity | P1 | medium | restricted | Desktop link checkbox; iPad runs input then output automatically. osf-gated. |
| Bulk multi-controller upload | menu/dialog | ✅ | ✅ | parity | P1 | hard | feasible | Desktop `MultiControllerUploadDialog`; iPad `bulkUploadControllersWithProgress` bridge loops `runUpload` over active open-source-firmware controllers + `BulkUploadSheet` progress/results sheet launched from the Controllers-tab "Upload All…" menu item. Closed firmware skipped. |
| Pixel test / test output | menu/dialog | ✅ | ❌ | ipad-missing | P1 | hard | infeasible | Desktop `PixelTestDialog` drives raw DMX/sACN/ArtNet/serial output. iOS sandbox blocks raw output → infeasible. |
| Remap DMX channels | dialog | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop DMX-effect panel → `RemapDMXChannelsDialog` (effect-level, edge of theme). No iPad DMX-remap UI. |
| FPP Connect (discover + per-FPP config + fseq upload) | menu/dialog | ✅ | ✅ | parity | P1 | hard | feasible | Desktop `FPPConnectDialog`; iPad Tools→FPP Connect `FPPConnectSheet` w/ parallel fan-out (`discoverFPPInstances`, `applyConfigToFPP`, `uploadFseq:toFPPInstances:`). Desktop dialog restyled 2026-07: instance list shades alternate rows, columns reordered (HostName/IP merged into dual links, "Pixel Hat/Cape" → "Upload Outputs"), sequence list shows subfolder-relative filenames w/ alternating shading + bold headers, loose fseqs found in subfolders, show-folder path label added. All desktop-layout/cosmetic; iPad FPPConnectSheet uses native SwiftUI list styling — no action needed. |
| FPP-proxy validation pre-upload warning | toolbar/dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `validateProxyForController` bridge wraps `FPP::ValidateProxy`; `startControllerUpload` runs it off-thread and surfaces a continue/cancel warning alert (`BulkAndProxyUploadModifier`) before any upload HTTP. |
| FPP Connect immediate-output upload for non-FPP discovered | menu/dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop uploads immediate output to non-FPP discovered devices (`src-ui-wx/controllers/FPPConnectDialog.cpp:1201` `bc->UploadForImmediateOutput`). iPad `applyConfigToFPP` non-FPP branch now creates a `BaseController` and calls `UploadForImmediateOutput` when the "Upload Controller Config" toggle is set and exactly one controller matches the device IP (`src-iPad/Bridge/XLSequenceDocument.mm`); the FPPConnectSheet surfaces that single toggle for non-FPP devices. |
| FPP Connect "Upload Controller" pushes input universes (#2747) | menu/dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop FPP Connect calls `inst->SetInputUniversesBridge(c.front())` (`src-ui-wx/controllers/FPPConnectDialog.cpp:1179`) in the upload-controller path (core `FPP::SetInputUniversesBridge`, `src-core/controllers/FPP.cpp:2176`). iPad `applyConfigToFPP` Cape branch now calls `target->SetInputUniversesBridge(matchedEth)` after the pixel/panel/serial/virtual-matrix uploads, gated on the same `uploadCape` ("Upload Controller") toggle + single matched controller (`src-iPad/Bridge/XLSequenceDocument.mm`). |
| Auto-upload on output-enable | dialog/panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad `startOutput` now sweeps AutoUpload-flagged active open-source-firmware controllers after `StartOutput()` and re-runs `runUpload` input+output (`src-iPad/Bridge/XLSequenceDocument.mm`). Closed firmware skipped silently. |
| Global output settings (Controller Sync, E1.31 Sync Universe, Global FPP Proxy, Global Force-Local-IP, Max-Suppress-Frames) | dialog/panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad `globalOutputSettings`/`setGlobalOutputSetting` bridge + `GlobalOutputSettingsView` shown in the Controllers tab when no controller is selected. Uses existing `OutputManager` accessors. |
| Vendor catalog browser | dialog | ✅ | ✅ | parity | P1 | hard | feasible | Desktop `VendorModelDialog` (LayoutPanel); iPad `VendorBrowserSheet` + `XLVendorCatalog`. |
| Map-from-Lights | dialog/wizard | ✅ | ✅ | parity | P1 | hard | feasible | Desktop macOS Continuity-Camera scan (`KLightMapperBridge`), now also Windows/Linux via the prebuilt KLightMapper library (`KLightMapperBridge_win.cpp`, local + RTSP/IP camera); iPad `MapFromLightsWizard` (FPP structured-light scan). Different mechanisms, same goal. |
| Controller capabilities query | other | ✅ | ✅ | parity | P1 | easy | feasible | Shared `ControllerCaps`; gates upload/visualize on both. |
| Falcon controller upload | toolbar | ✅ | ❌ | ipad-missing | P3 | hard | restricted | Closed firmware. IAP-gated P3 on iPad. |
| SanDevices upload | toolbar | ✅ | ❌ | ipad-missing | P3 | hard | restricted | Closed firmware. |
| HinksPix upload | toolbar | ✅ | ❌ | ipad-missing | P3 | hard | restricted | Closed firmware. |
| AlphaPix upload | toolbar | ✅ | ❌ | ipad-missing | P3 | hard | restricted | Closed firmware. |
| J1Sys upload | toolbar | ✅ | ❌ | ipad-missing | P3 | hard | restricted | Closed firmware. |
| Minleon upload | toolbar | ✅ | ❌ | ipad-missing | P3 | hard | restricted | Closed firmware. |
| Pixlite / Pixie upload | toolbar | ✅ | 🟡 | ipad-missing | P3 | hard | restricted | Open-FW path in-scope; vendor-FW path restricted. Mk3 config parse + port-array bounds fixed in `src-core/controllers/Pixlite16.cpp` (2026.14) — shared core, auto-applied if/when the iPad gains this upload path; no iPad UI work needed today. |
| FPP / WLED / ESPixelStick / DDP config + upload | dialog/toolbar | ✅ | ✅ | parity | P1 | medium | feasible | Open-source firmware — fully in-scope and present on iPad. |

## iPad gaps (desktop has, iPad missing)

### P1 — DONE

- **Bulk multi-controller upload.** ✅ **Shipped.**
  `bulkUploadControllersWithProgress:` on `XLSequenceDocument`
  (`src-iPad/Bridge/XLSequenceDocument.mm`) loops the shared
  `runUpload` (input+output) over every active open-source-firmware
  controller that supports upload, with a per-controller progress
  callback. The Controllers-tab "+" menu gains an **Upload All…**
  item that confirms, then drives `BulkUploadSheet`
  (`src-iPad/App/LayoutEditorView.swift`) — a live progress bar
  followed by a per-controller pass/fail list. Closed-firmware
  controllers are skipped by the bridge (restricted/IAP tier).

### P2 — DONE

- **Global output settings.** ✅ **Shipped.** `globalOutputSettings`
  getter + `setGlobalOutputSetting:value:` per-key setter on the
  bridge (over the existing `OutputManager` accessors), surfaced via
  `GlobalOutputSettingsView` in the Controllers tab when no
  controller is selected — Controller Sync, E1.31 Sync Universe
  (shown only when sync is on), Max Duplicate Frames To Suppress,
  Global Force Local IP, Global FPP Proxy.

- **Per-universe Output editing.** ✅ **Shipped.**
  `controllerPropertiesForName` now emits an **Output** section for
  E1.31/ArtNet/KiNET ethernet controllers — **Start Universe**,
  **Universe Count**, **Universe Per String**, **Individual Sizes**,
  a uniform **Channels per Universe** field (when not individual), or
  per-universe **Channels/<n>** fields (when individual). The matching
  `setControllerProperty` cases mirror desktop's adapter behaviour
  (renumbering, add/drop trailing outputs, `SetAllSameSize`).

- **Ping / controller health LED.** ✅ **Shipped.** `pingController:`
  wraps the shared core `Controller::Ping()` — on non-Windows hosts
  that's an HTTP reachability probe (`IPOutput::Ping` →
  `CurlManager::HTTPSGet`), so it runs inside the iOS sandbox without
  raw ICMP. A colour-coded status dot sits on each controller row and
  in the detail pane, tap-to-refresh, results cached per controller.

- **FPP-proxy validation pre-upload warning.** ✅ **Shipped.**
  `validateProxyForController:` wraps `FPP::ValidateProxy`;
  `startControllerUpload` runs it off the main thread and, on a bad
  proxy, presents an **Upload Anyway / Cancel** warning alert
  (`BulkAndProxyUploadModifier`) before any upload HTTP fires.

- **Auto-upload on output-enable.** ✅ **Shipped.** iPad `startOutput`
  now sweeps AutoUpload-flagged active open-source-firmware
  controllers after `StartOutput()` and re-runs input+output upload;
  closed-firmware controllers are skipped silently.

### P2 — remaining

- **FPP Connect immediate-output upload for non-FPP discovered
  devices.** ✅ **Shipped.** iPad `applyConfigToFPP`'s non-FPP branch
  now creates a `BaseController` and runs `UploadForImmediateOutput`
  when the per-device "Upload Controller Config" toggle is set and
  exactly one controller matches the device IP — mirroring desktop's
  `FPPConnectDialog.cpp:1199-1203`. The FPPConnectSheet shows that
  single toggle (instead of the FPP Models/UDP/Cape/Playlist set) for
  non-FPP devices.

- **Auto-upload on output-enable.** Desktop re-uploads controllers
  flagged auto-upload when output is enabled. iPad surfaces the
  `AutoUpload` property (`src-iPad/Bridge/XLSequenceDocument.mm:12336`,
  setter `:12607`) but `startOutput` calls bare `StartOutput()` without
  the auto-upload pass (`:9479`). **Work:** add an auto-upload sweep
  over flagged open-source-firmware controllers in the iPad
  output-enable path. **medium.**

### P3

- **Controller list sort menu** (by name/id/ip/proxy/vendor/protocol).
  ✅ **Shipped.** The iPad Controllers-tab header menu now has a Sort
  submenu (Name / Id / IP / Proxy / Vendor / Protocol) bridged through
  `sortControllers(byMode:)` → `OutputManager::SortControllersby*` —
  the same persistent core reorder desktop's `OnListControllerPopup`
  Sort submenu (`src-ui-wx/app-shell/TabSetup.cpp:2504`) invokes. Manual
  drag order still works too.
- **"Open Proxy" browser shortcut.** ✅ **Shipped.** The
  controller detail pane (`LayoutEditorControllerDetailView`) shows an
  **Open Proxy** button next to **Open** whenever the controller has
  an FPP proxy set; it opens `http://<proxy>/`.
- **Global "Export Controller Connections" XLSX.** ✅ **Shipped.** The
  iPad Controllers-tab header menu has an "Export Connections…" action
  → `exportControllerConnections(toPath:)`, which builds the same
  all-controllers libxlsxwriter workbook desktop's
  `OnMenuItem_ExportControllerConnectionsSelected`
  (`src-ui-wx/xLightsMain.cpp:8660`) does — per-controller merged
  header + `UDController::ExportAsCSV` rows + smart-remote shading — and
  hands it to `.fileExporter`. iPad exports the full field set rather
  than prompting (desktop shows a multi-choice field picker).
- **Force Local IP (per-controller)** — ✅ **Shipped.** `ForceLocalIP`
  enum descriptor + setter in the iPad ethernet property list, options
  sourced from `ip_utils::GetLocalIPs()` (index 0 = no override).
- **Remap DMX channels** — desktop DMX-effect-panel feature
  (`RemapDMXChannelsDialog`), at the edge of this theme. No iPad UI.
  **hard.**

## Desktop gaps (iPad has, desktop missing)

- **Controller filter / search (P3, easy).** iPad's Controllers tab
  has a name/vendor/model/IP search box
  (`filteredControllerRows`, `LayoutEditorView.swift:2463`). Desktop's
  controller list has no search — acceptable given the typically short
  list, but a quick add if asked.
- **Open-source-firmware badge (P3, easy).** iPad shows a green shield
  on osf controllers (`LayoutEditorView.swift:1884`). Desktop has no
  visual indicator that a controller's firmware is open-source.
- **Base-show link badge (P3, easy).** iPad shows a dedicated blue
  link icon for FromBase controllers. Desktop conveys FromBase through
  styling/colour rather than a discrete badge.

These are touch/UI-idiom niceties, not behavioral gaps; low priority to
backport.

## Infeasible / restricted on iPad

- **Pixel test / test output (infeasible).** `PixelTestDialog` drives
  raw sACN/ArtNet/DDP/serial-DMX output frames directly to wire. The
  iOS App Sandbox blocks raw multicast/broadcast UDP and all USB/serial
  access, so a faithful pixel-test cannot run from the iPad app. (FPP
  Connect uploads run over ordinary HTTP and are fine; live output is
  the blocked part.)
- **Raw serial / USB controllers (hardware-infeasible).** iPad has no
  USB-serial; serial controllers can be *configured/edited* (for shows
  that round-trip to desktop) but never *output to* from the iPad.
- **macOS Continuity-Camera scan path (platform-specific).** Desktop's
  Map-from-Lights uses a paired iPhone camera via `KLightMapperBridge`.
  iPad reaches the same goal through its own FPP structured-light
  `MapFromLightsWizard`; the camera-bridge path itself is desktop-only
  and not needed on iPad.
- **Closed-firmware vendor uploads (restricted — IAP-gated, P3):**
  Falcon, SanDevices, HinksPix, AlphaPix, J1Sys, Minleon, and the
  vendor-firmware Pixlite path. Per the firmware policy these are
  in-scope only as a restricted/paid tier; open-source firmware
  (FPP / WLED / ESPixelStick / DDP / generic / Kulp / open Pixlite)
  upload+config is already shipping and unrestricted.

## Recommended sequencing

Steps 1–4 below plus the per-controller Force-Local-IP / "Open Proxy" /
FPP-proxy-validation / auto-upload-on-enable items have **all shipped**
(see the DONE sections above). What's left:

1. ~~Bulk multi-controller upload (P1).~~ ✅ Done.
2. ~~Global output settings (P2).~~ ✅ Done.
3. ~~Per-universe Output editing (P2).~~ ✅ Done.
4. ~~Ping / health LED (P2).~~ ✅ Done.
5. ~~P3 polish: controller sort menu, global
   Export-Controller-Connections XLSX, FPP Connect immediate-output
   upload for non-FPP discovered devices, FPP Connect input-universe
   push (#2747).~~ ✅ Done.
6. **Restricted tier (P3, deferred):** closed-firmware vendor uploads
   behind IAP once the open-firmware experience is fully solid.
