# 07 · Setup, Controllers, Outputs & Upload

> **As of PR #6690 (2026-07) the desktop matches the iPad's surface**: the
> standalone Setup tab is gone and the controller list now lives on a
> Controllers page of the Layout tab (`src-ui-wx/layout/ControllerListPanel.cpp`).
>
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
| Controller layout placement box (Show on Layout) | dialog/panel + preview | ✅ | ✅ | parity | P3 | medium | feasible | A controller can be shown as a movable box in the layout preview, placed where it physically sits. Per-controller visibility (Off / Controller Tab Only / Layout Panel / Always, default Off). Object + visibility policy are wx-free core (`ControllerObject::ShouldDraw`), shared by both. Desktop: `ControllerListPanel::SetControllerObjectVisibility` + tree context menu; `ModelPreview::ShouldDrawViewObject`. iPad: `LayoutVisibility` descriptor in `controllerPropertiesForName` / `setControllerProperty`, `-[XLMetalBridge shouldDrawViewObject:context:]` + `setControllersTabActive:`, canvas pick routed to the Controllers tab via `controllerName(forViewObject:)`, box anchored as the drag target via `controllerObjectName(forController:)`. **iPad is ahead here**: its view-object draw loop isn't gated on `Is3D()`, so boxes work in 2D and 3D; desktop is 3D-only until the 2D view-object editing work in §7 of `plans/controller-layout-objects.md` lands. |
| "Models not on Controller" preview highlight (#6779) | context-menu + preview | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop preview right-click menu (Controllers tab only) selects every non-shadow model with no assigned controller (`Model::GetControllerName().empty()`, `LayoutPanel::OnPreviewRightDown`/`OnPreviewModelPopup` at `ID_PREVIEW_MODELS_NOT_ON_CONTROLLER`, `src-ui-wx/layout/LayoutPanel.cpp`); those models render orange (`ColorManager::COLOR_MODEL_NOT_ON_CONTROLLER`) instead of the normal selection color while the Controllers tab is active, via `ModelPreview::RenderModels` gated on `ControllerObjectContext::LayoutEditorControllerTab` (`src-ui-wx/layout/ModelPreview.cpp`). No iPad Controllers-tab context menu or equivalent preview-highlight surface exists yet. |
| Last Input/Output Upload timestamp | dialog/panel | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop shows two read-only fields at the bottom of the property pane (`ControllerListPanel::UpdateControllerProperties`, `src-ui-wx/layout/ControllerListPanel.cpp`). Persisted via `GetXLightsConfig()` (JSON app settings, scoped by show directory via `MakeControllerTimestampKey` in `src-core/utils/UtilFunctions.h`) — no `NetworkChange()` call, so uploading never marks the show dirty. Recorded from both manual upload (`UploadInputToController`/`UploadOutputToController` in `src-ui-wx/app-shell/TabSetup.cpp`) and FPP Connect (`src-ui-wx/controllers/FPPConnectDialog.cpp`). `FormatTimestamp()` centralized in `src-core/utils/UtilFunctions.h`. iPad controller detail pane has no equivalent. |
| Controller IP (Ethernet) | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | Both editable; iPad shows in row secondary line. |
| Multicast toggle (Ethernet) | dialog/panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `Multicast` bool; locks IP field. |
| Ethernet output protocol (E1.31/ArtNet/DDP/ZCPP/…) | dialog/panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad `Protocol` enum from caps. ZCPP is deprecated and hidden from the no-caps fallback list on both (`GetDefaultEthernetTypes` / `EthernetProtocolOptions`) unless the controller already uses it, so it only appears for a vendor whose `.xcontroller` declares `zcpp` (Falcon, ESPixelStick). Selecting it warns on both: desktop is a Yes/No confirm that reverts the picker on No (`ControllerEthernetPropertyAdapter::HandlePropertyEvent`); iPad is a post-commit informational alert (`ControllerWarningAlertsModifier`) because the property sheet commits through the bridge before the view can veto. |
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
| Discovery (auto-detect controllers) | toolbar/menu | ✅ | ✅ | parity | P1 | medium | feasible | Desktop `ButtonDiscover` thread sweep; iPad "+"→Discover detached task → `runControllerDiscovery`. ZCPP is no longer probed on either platform — `ZCPPOutput::PrepareDiscovery` is deleted (shared/auto-applied; neither `PrepareAllControllerDiscovery` nor `runControllerDiscovery` had wired it since the xScanner split). |
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
| Multiple LED panel matrices per controller (mixed drivers) | dialog/sheet | ✅ | ✅ | parity | P2 | medium | feasible | A controller can run several panel matrices at once with different drivers — a cape shifting out its own panels while ColorLight receivers hang off the network. Protocols `LED Panel Matrix - Hat/Cap/Cape` / `- ColorLight` name which family a model targets (`PROTOCOL_LED_PANEL_MATRIX*`, `src-core/models/Pixels.h`); the port number is FPP's `panelMatrixID`, i.e. the number its LED Panels page shows on the tab. Core is **shared/auto-applied**: `FPP::UploadPanelOutputs` (`src-core/controllers/FPP.cpp`) matches on `panelMatrixID` and refuses to write when the entry's `subType` contradicts the protocol; `UDController::SetAllModelsToValidProtocols` deliberately exempts panel ports from `allsame` so a cape and a ColorLight matrix keep their own families. iPad calls the same `UploadPanelOutputs` (`XLSequenceDocument.mm`). iPad UI: `wiringForController` emits every panel port up to `caps->GetMaxLEDPanelMatrixPort()`, filling absent ones with `BuildEmptyPortEntry(@"ledPanelMatrix", …)` so an unpopulated matrix is a drop target (the Swift port section is kind-agnostic — any section with no models renders "Drop a model here"); `portCountsForController` reports `maxLEDPanelMatrixPort` and `MoveToPortSheet` gained an "LED Panel Matrices" section; the drop handler preserves an already-chosen driver family. |
| Upload input (universes) | toolbar/menu | ✅ | ✅ | parity | P1 | hard | restricted | iPad `uploadInputForController`, osf-gated. |
| Linked input+output upload | toolbar | ✅ | ✅ | parity | P1 | medium | restricted | Desktop link checkbox; iPad runs input then output automatically. osf-gated. |
| Bulk multi-controller upload | menu/dialog | ✅ | ✅ | parity | P1 | hard | feasible | Desktop `MultiControllerUploadDialog`; iPad `bulkUploadControllersWithProgress` bridge loops `runUpload` over active open-source-firmware controllers + `BulkUploadSheet` progress/results sheet launched from the Controllers-tab "Upload All…" menu item. Closed firmware skipped. |
| Pixel test / test output | menu/dialog | ✅ | 🟡 | ipad-weaker | P1 | hard | feasible | **Landed (Models + Controllers tabs)** — see [13-tools](13-tools-diagnostics-help.md). Network output (sACN/ArtNet/DDP) works on iPad under the multicast entitlement; only serial/USB is out of reach. |
| Remap DMX channels | dialog | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop DMX-effect panel → `RemapDMXChannelsDialog` (effect-level, edge of theme). No iPad DMX-remap UI. |
| Controller list docked in the Layout tab | tab/panel | ✅ | ✅ | parity | P2 | hard | feasible | **Desktop caught up to the iPad (2026-07, PR #6690).** The standalone Setup tab is gone; `ControllerListPanel` (`src-ui-wx/layout/ControllerListPanel.cpp`) is now a page of the Layout tab's notebook alongside Models/Groups/3D Objects, with the property grid in the AUI settings pane — the same shape the iPad has shipped since J-5 (`LayoutEditorView.swift`). Reverse-parity item in theme 14 (desktop-only surface) is now a *layout* difference only, not a capability one. |
| Controller → port → model tree expansion | tab/panel | ✅ | ✅ | parity | P2 | medium | feasible | Desktop `PopulateControllerPorts` (`src-ui-wx/layout/ControllerTreeUtils.cpp`) expands each controller into pixel/serial/PWM/virtual-matrix/LED-panel ports and the models on them, built from `UDController`. iPad shows the same decomposition in its Visualize/wiring sheet (`ControllerVisualizeView`). Different surface, same data. `UDController::Rescan`'s PWM branch dereferenced `Controller::GetControllerCaps()` unchecked, crashing whenever a PWM-protocol model sat on a controller with no capabilities definition; now null-guarded in shared core (`src-core/controllers/ControllerUploadData.cpp`), so **auto-applied to both platforms** — no iPad UI work. |
| Select a port/model row → highlight in layout preview | tab/panel | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `ModelPreview::SetPortStringHighlight`/`SetPortChannelHighlight` (`src-ui-wx/layout/ModelPreview.{h,cpp}`) dims every node except the selected port/string/model while a controller row is selected. iPad's wiring sheet has no companion preview highlight. Core-side helper `Model::GetNodePhysicalStringIndex` (`src-core/models/Model.h`, overridden in `CustomModel.h`) is **shared**, so only the SwiftUI/Metal preview side would need work. |
| Controller list column show/hide + saved order | tab/panel | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop right-click on the column header toggles any of the 17 controller columns, persisted per column title (`InstallTreeListColumnVisibility`, `src-ui-wx/shared/utils/TreeListColumnVisibility.cpp`); order persists in `LayoutControllerListCols`. Desktop-table idiom — the iPad Controllers tab is a SwiftUI list with a fixed row layout, so this would need a different design rather than a port. |
| "Show All Info" all-columns reference window | dialog | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop `ControllerFullColumnsDialog` (`src-ui-wx/layout/ControllerFullColumnsDialog.cpp`) — read-only view of every controller column for every controller, in a fixed curated order. No iPad equivalent; low value on a narrow screen. |
| FPP Connect scoped to a single controller | context-menu | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop controller context menu → "FPP Connect" opens `FPPConnectDialog` with a `targetIp`, which runs `FPP::PrepareSingleDiscovery` (`src-core/controllers/FPP.cpp`) instead of a full-show discovery and hides the "Add FPP" button. iPad's `FPPConnectSheet` always does a full discovery. The core-side `PrepareSingleDiscovery` is **shared**, so the iPad work is confined to plumbing a target IP through `discoverFPPInstances`. |
| FPP Connect (discover + per-FPP config + fseq upload) | menu/dialog | ✅ | ✅ | parity | P1 | hard | feasible | Desktop `FPPConnectDialog`; iPad Tools→FPP Connect `FPPConnectSheet` w/ parallel fan-out (`discoverFPPInstances`, `applyConfigToFPP`, `uploadFseq:toFPPInstances:`). Desktop dialog restyled 2026-07: instance list shades alternate rows, columns reordered (HostName/IP merged into dual links, "Pixel Hat/Cape" → "Upload Outputs"), sequence list shows subfolder-relative filenames w/ alternating shading + bold headers, loose fseqs found in subfolders, show-folder path label added. All desktop-layout/cosmetic; iPad FPPConnectSheet uses native SwiftUI list styling — no action needed. Universe-outputs handling (`FPP::UploadUDPOut` / `FPP::CreateUniverseFile`, `src-core/controllers/FPP.cpp`, FPP 10+ only) now carries forward the per-controller UDP pacing/bandwidth caps already set on the FPP (keyed by destination IP) plus the output-level global pacing default, instead of wiping them when regenerating the file; the optional `<MaxPacing>` controller cap (`ControllerCaps::GetMaxPacing`) is written authoritatively when the controller is under full xLights control, otherwise seeds new entries only (existing FPP value wins). **Shared core, auto-applied to both platforms.** |
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
| Pixlite / Pixie upload | toolbar | ✅ | 🟡 | ipad-missing | P3 | hard | restricted | Open-FW path in-scope; vendor-FW path restricted. Mk3 config parse + port-array bounds fixed in `src-core/controllers/Pixlite16.cpp` (2026.14) — shared core, auto-applied if/when the iPad gains this upload path; no iPad UI work needed today. 2026-07-29: `SetOutputs` also bails when `ControllerCaps::GetControllerConfig` returns nullptr (no caps definition for the vendor/model/variant) instead of dereferencing it for the port-count clamps — shared core, auto-applied. |
| FPP / WLED / ESPixelStick / DDP config + upload | dialog/toolbar | ✅ | ✅ | parity | P1 | medium | feasible | Open-source firmware — fully in-scope and present on iPad. |
| Controller layout print preview | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop right-click "Print…" on the controller layout now opens a resizable print-preview dialog (adjustable box size, fit-to-page) before printing, replacing the direct-to-printer entry (`src-ui-wx/setup/ControllerModelDialog.cpp`). iPad has no print path for the visualizer; UIKit's `UIPrintInteractionController` + a rendered page would be the analogue. |
| Controller list: show / hide columns from the header | panel | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop lets the controller list's columns be shown/hidden from the header context menu (landed with the Controllers-into-Layout move). iPad's controller list has a fixed column set — no `controllerColumns`/`visibleColumns` state in `src-iPad/`. |
| Remove "Keep Channel Numbers" for FPP devices | property | ✅ | ✅ | parity | P3 | easy | feasible | Desktop #5459 drops the `KeepChannelNumbers` property for FPP-type controllers (`ControllerPropertyAdapter.cpp:489`, `OutputPropertyAdapters.cpp:437/454`). The property list the iPad renders comes from the same shared controller/output model, and `src-iPad/` has no `KeepChannelNumbers` reference of its own, so the removal is inherited. Verify the iPad property pane no longer offers it for FPP controllers. |
| Sync smart-remote type across a port block | property | ✅ | 🟡 | ipad-weaker | P3 | medium | feasible | Desktop #3468. **Resolved:** the *capability* is shared core — `ControllerCaps::AllSmartRemoteTypesPerPortMustBeSame()` — but the propagation loop that acts on it is desktop UI (`src-ui-wx/setup/ControllerModelDialog.cpp:800-820`: when the cap is set, walk all four ports from `GetBasePort()` and write `SetSmartRemote(1)` + `USE_SMART_REMOTE` + `SetSmartRemoteType` on every model). So the iPad does **not** inherit it — `ModelSmartRemoteSheet` sets the one model. Work: replicate that loop against the same cap. |
| KulpLights dual serial ports: independent protocols | property | ✅ | ✅ | parity | P3 | easy | feasible | Desktop #3926 lets KulpLights controllers with two serial ports use a different protocol per port. The capability is declared by the vendor controller definitions in shared `src-core/controllers/`, so the iPad's port editors pick it up with no iPad-side change. |
| ESPixelStick: new ESP32 variants, TLS3001, corrected port channel caps | vendor data | ✅ | ✅ | parity | P3 | easy | feasible | Desktop added the Octa2Go and other ESP32 variants, TLS3001 on V4, and fixed max pixel-port channel counts. Vendor capability data lives in shared `src-core/controllers/`, so the iPad inherits the new variants and caps automatically — no iPad UI work. |
| WLED auto-discovery via mDNS (`_wled._tcp`) | discovery | ✅ | ✅ | parity | P2 | easy | feasible | Desktop discovers WLED devices over mDNS and adds them as DDP with auto-size/auto-layout. Discovery is shared wx-free `src-core/discovery/`, so the iPad's discovery sheet gets the same results; only the Windows-native DNS-SD backend (below) is platform-specific. |
| FPP discovery over mDNS on Windows (native windns.h) | discovery | ✅ | ➖ | n/a | P3 | easy | feasible | Desktop added native `windns.h` DNS-SD to complement broadcast/multicast ping — a Windows-only backend for the shared discovery API. Apple platforms already had mDNS, so there is nothing for the iPad to mirror. |
| FPP Connect: preserve per-controller UDP pacing / bandwidth caps | dialog | ✅ | ✅ | parity | P3 | easy | feasible | **Resolved:** `MaxPacing` exists only in shared core — `src-core/controllers/ControllerCaps.{h,cpp}` and `src-core/controllers/FPP.cpp`, with **no** `src-ui-wx` references — so preserving the caps configured on FPP10 while regenerating the universe outputs file is inherited by the iPad's FPP upload path unchanged. |
| FPP Connect: upload inputs the way controller upload does | dialog | ✅ | ✅ | parity | P3 | easy | feasible | Desktop #2747 aligns FPP Connect's input upload with the controller-upload path. **Resolved:** that path is `FPP::SetInputUniverses` / `SetInputUniversesBridge` in shared `src-core/controllers/FPP.cpp:2019,2275`, with no matching logic in the desktop `FPPConnectDialog` — so the iPad's FPP upload inherits the aligned behaviour. |
| FPP "Player Only" also sets "xLights Only" | dialog | ✅ | 🟡 | ipad-weaker | P3 | easy | feasible | Desktop #6453 couples the two modes. **Resolved:** unlike the two rows above this one is *not* inherited — `src-core/outputs/Controller.cpp` has no coupling; it lives in the desktop property/list UI (`src-ui-wx/controllerproperties/ControllerPropertyAdapter.cpp`, `src-ui-wx/layout/ControllerListPanel.cpp`, `src-ui-wx/app-shell/TabSetup.cpp`). The iPad needs the same coupling wherever it exposes the two flags. |

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
