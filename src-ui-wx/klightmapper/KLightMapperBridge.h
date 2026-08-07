#pragma once

// Plain-C++ façade over KLightMapper. Lets src-ui-wx call into KLightMapper
// without dragging Swift / ObjC / engine headers into wx-side translation
// units.
//
// One implementation is linked per desktop platform: KLightMapperBridge.mm on
// macOS (wraps the Swift @objc KLM via the framework), KLightMapperBridge_win.cpp
// on Windows + Linux (calls the prebuilt library's flat-C ABI, klm/scan_api.h).
// KLightMapper is a required, auto-fetched dependency on every desktop, so call
// sites use these symbols unconditionally (no platform gating needed).

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

/// A network (ONVIF/IP) camera found on the LAN via WS-Discovery.
struct NetworkCamera {
    std::string name;            // friendly name (model), else host
    std::string host;            // IPv4 address
    std::string deviceService;   // ONVIF device-service URL
};

/// One H.264 stream profile of a network camera.
struct CameraProfile {
    std::string name;            // ONVIF profile name
    std::string rtspUrl;         // rtsp:// stream URL
    int         width = 0;
    int         height = 0;
    int         fps = 0;
};

/// Discover ONVIF cameras on the local network (WS-Discovery, UDP
/// multicast). **Blocks** up to `timeoutMs` (~2–3 s) — call off the UI
/// thread. The picker presents these alongside Continuity cameras.
std::vector<NetworkCamera> DiscoverNetworkCameras(int timeoutMs);

/// Enumerate a discovered camera's H.264 profiles (resolution + RTSP
/// URL), sorted largest-resolution first (profile[0] is the best
/// stream). `user`/`pass` are the camera's ONVIF credentials (may be
/// empty). Blocks on the network — call off the UI thread.
std::vector<CameraProfile> EnumerateCameraProfiles(
    const std::string& deviceService,
    const std::string& user,
    const std::string& pass);

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

/// Open the macOS scan window using a remote RTSP/IP camera as the
/// frame source instead of a local AVCapture device (a Mac with no
/// usable built-in / Continuity camera can drive a scan from a
/// networked camera). FFmpeg-free — KLightMapper decodes the stream
/// with Network.framework + VideoToolbox. The user still connects to
/// their FPP controller in the window's Connect step; only the camera
/// differs.
///
/// `rtspURL` is the full stream URL (e.g. `rtsp://host:554/stream1`).
/// `username` / `password` may be empty (pass empty string for none);
/// credentials embedded in the URL userinfo also work. `scanDumpParent`
/// behaves exactly as in `PresentScanWindow`. Hands back the produced
/// .xmodel path (or std::nullopt on cancel / failure / bad URL /
/// unsupported platform) via `completion` on the main thread.
void PresentRTSPScanWindow(
    const std::string& rtspURL,
    const std::string& username,
    const std::string& password,
    const std::string& scanDumpParent,
    std::function<void(std::optional<std::string>)> completion);

} // namespace klbridge
