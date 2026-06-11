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
> fseq fan-out). The genuine remaining iPad gaps are: **bulk
> multi-controller upload**, **global output settings** (Controller
> Sync / E1.31 Sync Universe / Global FPP Proxy / Global Force-Local-IP /
> Max-Suppress-Frames), **per-universe Output editing** (Universe #,
> Universes count, UniversePerString, IndivSizes, per-universe
> channels), the **global "Export Controller Connections" XLSX**, an
> **"Open Proxy" browser** shortcut, **ping/health LED**, and **pixel
> test** (infeasible — raw DMX/sACN/ArtNet output is sandbox-blocked).
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
| Full xLights Control + default brightness/gamma | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `FullxLightsControl` / `DefaultBrightnessUnderFullxLightsControl` / `DefaultGammaUnderFullxLightsControl`. |
| Suppress duplicate frames | dialog/panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad `SuppressDuplicates` bool. |
| Monitor | dialog/panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad `Monitor` bool. |
| Vendor / Model / Variant cascade | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | Shared `ControllerCaps::GetVendors/Models/Variants`; iPad enum cascade. |
| Caps "extra" properties | dialog/panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad `ControllerExtra.*` enum/string from `GetExtraPropertyDefs()`. |
| Last Input/Output Upload timestamp | dialog/panel | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop shows two read-only fields at the bottom of the property pane (`Controllers_PropertyEditor` in `src-ui-wx/app-shell/TabSetup.cpp`). Timestamps recorded via `Controller::SetExtraProperty` from both manual upload (`UploadInputToController`/`UploadOutputToController` at `src-ui-wx/app-shell/TabSetup.cpp`) and FPP Connect (`src-ui-wx/controllers/FPPConnectDialog.cpp`). `FormatTimestamp()` centralized in `src-core/utils/UtilFunctions.h`. iPad controller detail pane has no equivalent. |
| Controller IP (Ethernet) | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | Both editable; iPad shows in row secondary line. |
| Multicast toggle (Ethernet) | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `Multicast` bool; locks IP field. |
| Ethernet output protocol (E1.31/ArtNet/DDP/ZCPP/…) | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad `Protocol` enum from caps. |
| Priority (Ethernet) | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `Priority` int 0..100. |
| Managed flag (Ethernet) | dialog/panel | ✅ | ✅ | parity | P3 | easy | feasible | Read-only on both. |
| Per-controller FPP Proxy IP | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `FPPProxy` string (Ethernet + FPP-serial). |
| Force Local IP (per-controller) | dialog/panel | ✅ | 🟡 | ipad-missing | P3 | easy | feasible | Desktop `ForceLocalIP` enum; iPad property list does not surface it. |
| Universe number / Universes count | dialog/panel | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop ethernet adapter `Universe`/`Universes`/`UniversePerString`/`IndivSizes`/per-universe `Channels`. iPad shows only a read-only channel-range summary. |
| Serial port selection | dialog/panel | ✅ | 🟡 | parity | P1 | easy | feasible | Desktop port dropdown; iPad enumerates system ports or freeform (no hw serial on iPad) + FPP `tty/i2c/spi` ports. |
| Serial output protocol (DMX/LOR/Renard/Minleon/…) | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad `SerialProtocolOptions`. |
| Serial baud / speed | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad `SerialBaudOptions`, grey-out when protocol fixes baud. |
| Serial i2c / spi / generic prefix-postfix | dialog/panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad covers i2c device, SPI kHz, generic-serial prefix/postfix. |
| Serial channels (auto-size aware) | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `Channels` int, read-only when AutoSize. |
| Null controller channels | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `Channels` 1..1000000. |
| Models-on-controller list / channel range | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop "Models" prop + range; iPad detail-pane member list, tap to select. |
| Discovery (auto-detect controllers) | toolbar/menu | ✅ | ✅ | parity | P1 | medium | feasible | Desktop `ButtonDiscover` thread sweep; iPad "+"→Discover detached task → `runControllerDiscovery`. |
| Discovery mismatch resolution (IP/name) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop inline `wxMessageDialog` prompts (TabSetup ~1569/1612); iPad dedicated `DiscoveryMismatchModifier` sheet → `applyDiscoveryMismatch`. |
| Discovery auth (401 credentials) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop `DiscoveryAuthDialog`; iPad `setFPPAuthPromptHandler` + `FPPAuthPrompt`. |
| Controller filter / search | tab | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad name/vendor/model/IP search; desktop has no search box (small list). |
| Controller list sort (by name/id/ip/proxy/vendor/protocol) | context-menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop "Sort" submenu; iPad only manual drag order. |
| Controller list column sort (header click) | gesture | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `OnListControllersColClick`; iPad list isn't column-based. |
| Open browser to controller | context-menu/gesture | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `ButtonOpen` / double-click; iPad double-tap row + context-menu "Open <ip>" + detail button → `UIApplication.shared.open`. |
| Open FPP Proxy in browser | toolbar | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop `Button_OpenProxy` opens proxy http page; iPad has no proxy-open shortcut (per-controller proxy IS editable, just no "open" button). |
| Ping / health LED | toolbar | ✅ | ❌ | ipad-missing | P2 | easy | feasible | Desktop `LedPing` colour-codes last ping. No iPad bridge ping method/UI; ICMP from iOS app is constrained but TCP-port probe is feasible. |
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
| Export Controller Connections (global XLSX) | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `OnMenuItem_ExportControllerConnectionsSelected` → libxlsxwriter all-controllers workbook. iPad only per-controller CSV/JSON. |
| Upload output (wiring) | toolbar/menu | ✅ | ✅ | parity | P1 | hard | restricted | iPad `uploadOutputForController`, gated to open-source firmware (osf). Closed FW = restricted P3. |
| Upload input (universes) | toolbar/menu | ✅ | ✅ | parity | P1 | hard | restricted | iPad `uploadInputForController`, osf-gated. |
| Linked input+output upload | toolbar | ✅ | ✅ | parity | P1 | medium | restricted | Desktop link checkbox; iPad runs input then output automatically. osf-gated. |
| Bulk multi-controller upload | menu/dialog | ✅ | ❌ | ipad-missing | P1 | hard | feasible | Desktop `MultiControllerUploadDialog` (all controllers, progress). iPad has no bulk-upload UI. |
| Pixel test / test output | menu/dialog | ✅ | ❌ | ipad-missing | P1 | hard | infeasible | Desktop `PixelTestDialog` drives raw DMX/sACN/ArtNet/serial output. iOS sandbox blocks raw output → infeasible. |
| Remap DMX channels | dialog | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop DMX-effect panel → `RemapDMXChannelsDialog` (effect-level, edge of theme). No iPad DMX-remap UI. |
| FPP Connect (discover + per-FPP config + fseq upload) | menu/dialog | ✅ | ✅ | parity | P1 | hard | feasible | Desktop `FPPConnectDialog`; iPad Tools→FPP Connect `FPPConnectSheet` w/ parallel fan-out (`discoverFPPInstances`, `applyConfigToFPP`, `uploadFseq:toFPPInstances:`). |
| FPP-proxy validation pre-upload warning | toolbar/dialog | ✅ | ❌ | ipad-missing | P2 | easy | feasible | Desktop calls `FPP::ValidateProxy(...)` before upload (`src-ui-wx/app-shell/TabSetup.cpp:2692`, `:2718`; core `src-core/controllers/FPP.h:159` / `FPP.cpp:4199`). iPad has no pre-upload proxy-validation warning. |
| FPP Connect immediate-output upload for non-FPP discovered | menu/dialog | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop uploads immediate output to non-FPP discovered devices (`src-ui-wx/controllers/FPPConnectDialog.cpp:1201` `bc->UploadForImmediate`). iPad early-returns when `fppType != FPP` (`src-iPad/Bridge/XLSequenceDocument.mm:14503-14507`). |
| Auto-upload on output-enable | dialog/panel | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop re-uploads auto-upload controllers when output is enabled. iPad exposes the `AutoUpload` prop (`src-iPad/Bridge/XLSequenceDocument.mm:12336`, setter `:12607`) but `startOutput` calls bare `StartOutput()` with no auto-upload pass (`:9479`). |
| Global output settings (Controller Sync, E1.31 Sync Universe, Global FPP Proxy, Global Force-Local-IP, Max-Suppress-Frames) | dialog/panel | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop "nothing selected" property grid. iPad never surfaces these global toggles. |
| Vendor catalog browser | dialog | ✅ | ✅ | parity | P1 | hard | feasible | Desktop `VendorModelDialog` (LayoutPanel); iPad `VendorBrowserSheet` + `XLVendorCatalog`. |
| Map-from-Lights | dialog/wizard | ✅ | ✅ | parity | P1 | hard | feasible | Desktop macOS Continuity-Camera scan (`KLightMapperBridge`); iPad `MapFromLightsWizard` (FPP structured-light scan). Different mechanisms, same goal. |
| Controller capabilities query | other | ✅ | ✅ | parity | P1 | easy | feasible | Shared `ControllerCaps`; gates upload/visualize on both. |
| Falcon controller upload | toolbar | ✅ | ❌ | ipad-missing | P3 | hard | restricted | Closed firmware. IAP-gated P3 on iPad. |
| SanDevices upload | toolbar | ✅ | ❌ | ipad-missing | P3 | hard | restricted | Closed firmware. |
| HinksPix upload | toolbar | ✅ | ❌ | ipad-missing | P3 | hard | restricted | Closed firmware. |
| AlphaPix upload | toolbar | ✅ | ❌ | ipad-missing | P3 | hard | restricted | Closed firmware. |
| J1Sys upload | toolbar | ✅ | ❌ | ipad-missing | P3 | hard | restricted | Closed firmware. |
| Minleon upload | toolbar | ✅ | ❌ | ipad-missing | P3 | hard | restricted | Closed firmware. |
| Pixlite / Pixie upload | toolbar | ✅ | 🟡 | ipad-missing | P3 | hard | restricted | Open-FW path in-scope; vendor-FW path restricted. |
| FPP / WLED / ESPixelStick / DDP config + upload | dialog/toolbar | ✅ | ✅ | parity | P1 | medium | feasible | Open-source firmware — fully in-scope and present on iPad. |

## iPad gaps (desktop has, iPad missing)

### P1

- **Bulk multi-controller upload.** Desktop:
  `xLightsFrame::OnMenuItemBulkControllerUploadSelected`
  (`src-ui-wx/xLightsMain.cpp:8647`) opens
  `MultiControllerUploadDialog` (`src-ui-wx/setup/MultiControllerUploadDialog.cpp`)
  which uploads input+output to every controller with progress
  tracking. iPad only uploads a single controller at a time from the
  Controllers-tab context menu / detail pane
  (`startControllerUpload` in `LayoutEditorView.swift:1315`). **Work:**
  a `bulkUploadAllControllers` bridge method on `XLSequenceDocument`
  that loops `uploadInputForController`/`uploadOutputForController`
  over open-source-firmware controllers, plus a SwiftUI progress sheet
  driven off the per-controller results. Core upload logic already
  shared — this is bridge + UI only. **medium-hard.**

### P2

- **Global output settings.** Desktop renders these in the property
  grid when nothing/multiple controllers are selected
  (`src-ui-wx/app-shell/TabSetup.cpp:1957-1983`): **Controller Sync**,
  **E1.31 Sync Universe**, **Max Duplicate Frames To Suppress**,
  **Global Force Local IP**, **Global FPP Proxy**. None are surfaced on
  iPad. **Work:** `globalOutputSettings` getter + per-key setter on the
  bridge (the `OutputManager` accessors `GetSyncUniverse`/`SetSync*` /
  `GetGlobalFPPProxy` / `GetGlobalForceLocalIP` / `GetSuppressFrames`
  already exist), plus a SwiftUI "Global" section in the Controllers tab
  shown when no controller is selected. **medium.**

- **Per-universe Output editing.** Desktop's
  `ControllerEthernetPropertyAdapter` exposes **Universe** number,
  **Universes** count, **UniversePerString**, **IndivSizes**, and
  per-universe **Channels** for E1.31/ArtNet
  (`src-ui-wx/controllerproperties/ControllerEthernetPropertyAdapter.cpp:469-525`).
  iPad's `controllerPropertiesForName`
  (`src-iPad/Bridge/XLSequenceDocument.mm:12381-12402`) shows only the
  protocol + a read-only channel-range summary. **Work:** add the
  universe-tree props to the bridge property list and an expandable
  Output section in the detail pane. **medium.**

- **Ping / controller health LED.** Desktop colour-codes
  `LedPing` from `controller->GetLastPingState()`
  (`src-ui-wx/app-shell/TabSetup.cpp:2031-2040`). iPad has no ping
  bridge method or status indicator. **Work:** `pingController` bridge
  wrapper (TCP-port reachability probe — raw ICMP is constrained on
  iOS) + a status dot on the controller row / detail pane. **easy.**

- **FPP-proxy validation pre-upload warning.** Desktop validates the
  FPP proxy before upload via `FPP::ValidateProxy(...)`
  (`src-ui-wx/app-shell/TabSetup.cpp:2692`, `:2718`; core
  `src-core/controllers/FPP.h:159` / `FPP.cpp:4199`) and warns on a bad
  proxy. iPad runs no equivalent pre-upload check. **Work:** call the
  shared `ValidateProxy` from the iPad upload path and surface a warning
  alert. **easy.**

- **FPP Connect immediate-output upload for non-FPP discovered
  devices.** Desktop pushes an immediate output config to non-FPP
  devices found during FPP Connect discovery
  (`src-ui-wx/controllers/FPPConnectDialog.cpp:1201`
  `bc->UploadForImmediate`). iPad early-returns when the discovered
  device's `fppType != FPP`
  (`src-iPad/Bridge/XLSequenceDocument.mm:14503-14507`), skipping those
  devices. **Work:** extend the iPad FPP Connect upload to handle the
  non-FPP immediate-output branch. **medium.**

- **Auto-upload on output-enable.** Desktop re-uploads controllers
  flagged auto-upload when output is enabled. iPad surfaces the
  `AutoUpload` property (`src-iPad/Bridge/XLSequenceDocument.mm:12336`,
  setter `:12607`) but `startOutput` calls bare `StartOutput()` without
  the auto-upload pass (`:9479`). **Work:** add an auto-upload sweep
  over flagged open-source-firmware controllers in the iPad
  output-enable path. **medium.**

### P3

- **Controller list sort menu** (by name/id/ip/proxy/vendor/protocol),
  desktop `OnListControllerPopup` "Sort" submenu
  (`src-ui-wx/app-shell/TabSetup.cpp:2497-2504`) + header-click sort.
  iPad only supports manual drag order. **medium.**
- **"Open Proxy" browser shortcut.** Desktop `Button_OpenProxy`.
  Per-controller FPP proxy is already editable on iPad; only the
  one-tap "open the proxy's web page" button is missing. **easy.**
- **Global "Export Controller Connections" XLSX.** Desktop
  `OnMenuItem_ExportControllerConnectionsSelected`
  (`src-ui-wx/xLightsMain.cpp:8660`) writes an all-controllers
  libxlsxwriter workbook. iPad has only per-controller CSV/JSON from
  the Visualize sheet. The iPad already has a libxlsxwriter path
  (`exportModelsReport`) to model from. **medium.**
- **Force Local IP (per-controller)** — desktop ethernet prop not in
  the iPad property list. **easy.**
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

1. **Bulk multi-controller upload (P1).** Highest user value; core
   upload logic is already shared, so this is a bridge loop + a
   progress sheet. Restrict to open-source firmware to match the
   single-controller gate.
2. **Global output settings (P2).** Small, self-contained: surface
   Controller Sync / E1.31 Sync Universe / Global FPP Proxy / Global
   Force-Local-IP / Max-Suppress-Frames in a "Global" section shown
   when no controller is selected. All `OutputManager` accessors exist.
3. **Per-universe Output editing (P2).** Extend
   `controllerPropertiesForName` with the universe tree so E1.31/ArtNet
   controllers are fully configurable, not just protocol-pickable.
4. **Ping / health LED (P2).** TCP-reachability probe + a status dot;
   cheap and improves the "is my controller online" loop on-device.
5. **P3 polish:** controller sort menu, "Open Proxy" button, global
   Export-Controller-Connections XLSX, Force-Local-IP per-controller.
6. **Restricted tier (P3, deferred):** closed-firmware vendor uploads
   behind IAP once the open-firmware experience is fully solid.
