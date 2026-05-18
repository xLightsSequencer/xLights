#pragma once

// Plain-C++ façade over KLightMapper's ObjC API (`@objc KLM`,
// `@objc KLMCameraInfo`). Lets src-ui-wx call into KLightMapper
// without dragging Swift / ObjC headers into wx-side translation
// units.
//
// Implementation (KLightMapperBridge.mm) compiles only on macOS;
// call sites in cross-platform code must gate on __APPLE__ so
// non-Apple builds don't try to link these symbols.

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace klbridge {

struct CameraInfo {
    std::string identifier;       // AVCaptureDevice.uniqueID — round-trips to PresentScanWindow
    std::string localizedName;    // user-facing label
    bool        isContinuityCamera = false;
};

/// Snapshot of Continuity Cameras currently visible to the Mac.
/// Returns an empty list on iOS, pre-macOS-14, or when no iPhone is
/// paired — callers should treat empty as "no camera-scan option."
std::vector<CameraInfo> DiscoverContinuityCameras();

/// Open the macOS camera-scan window for the chosen camera. Hands
/// back the produced .xmodel path (or std::nullopt on cancel /
/// failure / unsupported platform) via `completion`, dispatched on
/// the main thread.
///
/// `scanDumpParent` (optional, pass empty string to skip) is the
/// directory under which the engine persists raw scan artefacts
/// — `<scanDumpParent>/scan_<timestamp>/{scan.mov, *.png, state.json}`.
/// xLights desktop passes the show folder's `MapFromLightsDebug/`
/// here so failed multi-view solves and underdetected scans can be
/// reproduced offline. When empty, the analyzer still runs but the
/// dump isn't retained.
void PresentScanWindow(
    const std::string& cameraID,
    const std::string& scanDumpParent,
    std::function<void(std::optional<std::string>)> completion);

} // namespace klbridge
