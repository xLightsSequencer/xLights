// KLightMapperBridge_win.cpp — the desktop (Windows + Linux) backend for the
// klbridge:: facade declared in KLightMapperBridge.h. The macOS counterpart
// is the ObjC++ KLightMapperBridge.mm (which wraps the Swift @objc KLM). This
// one calls the prebuilt library's C ABI (klm/scan_api.h) — klightmapper.dll on
// Windows, libklightmapper.so on Linux. The boundary is identical on both, and
// this file is pure portable C++ + wx, so one backend serves both.
//
// Lives in src-ui-wx/klightmapper/ so the Windows/Linux CMake GLOB_RECURSE picks
// it up; the macOS Xcode project builds the sibling .mm instead, so they never
// collide. The _WIN32||__linux__ guard compiles it to nothing on macOS, where
// the .mm provides klbridge. KLightMapper is a required, auto-fetched dependency
// (the build systems download the lib + headers), so there is no "disabled"
// path — no XLIGHTS_HAVE_KLIGHTMAPPER.
#if defined(_WIN32) || defined(__linux__)

#include "KLightMapperBridge.h"

#include "klm/scan_api.h"

// MSVC: pull in the import lib for klightmapper.dll. lib\windows64 (which holds
// klightmapper.lib) is already on the VS project's LibraryPath. Linux links it
// via -lklightmapper from build_scripts/linux/klightmapper.mak instead; other
// compilers ignore the unknown pragma (Linux builds pass -Wno-unknown-pragmas).
#ifdef _WIN32
#pragma comment(lib, "klightmapper.lib")
#endif

#include <wx/app.h>   // wxTheApp->CallAfter — marshal completion to the UI thread

#include <vector>

namespace klbridge {

std::vector<CameraInfo> DiscoverContinuityCameras() {
    // On Windows "continuity cameras" == the local Media Foundation webcams the
    // picker offers for a local-camera scan. isContinuityCamera stays false.
    std::vector<CameraInfo> out;
    klm_camera_list* list = klm_scan_camera_list();
    const int n = klm_camera_list_count(list);
    out.reserve((size_t)n);
    for (int i = 0; i < n; ++i) {
        CameraInfo ci;
        ci.identifier = klm_camera_list_id(list, i);
        ci.localizedName = klm_camera_list_name(list, i);
        ci.isContinuityCamera = false;
        out.push_back(std::move(ci));
    }
    klm_camera_list_free(list);
    return out;
}

std::vector<NetworkCamera> DiscoverNetworkCameras(int timeoutMs) {
    std::vector<NetworkCamera> out;
    constexpr int kCap = 32;
    std::vector<klm_net_camera> tmp((size_t)kCap);
    const int n = klm_discover_network_cameras(tmp.data(), kCap, timeoutMs);
    for (int i = 0; i < n && i < kCap; ++i) {
        NetworkCamera nc;
        nc.name = tmp[i].name;
        nc.host = tmp[i].host;
        nc.deviceService = tmp[i].device_service;
        out.push_back(std::move(nc));
    }
    return out;
}

std::vector<CameraProfile> EnumerateCameraProfiles(const std::string& deviceService,
                                                   const std::string& user,
                                                   const std::string& pass) {
    std::vector<CameraProfile> out;
    constexpr int kCap = 16;
    std::vector<klm_net_profile> tmp((size_t)kCap);
    char err[256] = {0};
    const int n = klm_enumerate_camera_profiles(
        deviceService.c_str(),
        user.empty() ? nullptr : user.c_str(),
        pass.empty() ? nullptr : pass.c_str(),
        tmp.data(), kCap, err, (int)sizeof(err));
    for (int i = 0; i < n && i < kCap; ++i) {
        CameraProfile cp;
        cp.name = tmp[i].name;
        cp.rtspUrl = tmp[i].rtsp_url;
        cp.width = tmp[i].width;
        cp.height = tmp[i].height;
        cp.fps = tmp[i].fps;
        out.push_back(std::move(cp));
    }
    return out;
}

namespace {

// The std::function completion can't cross the C ABI, so we heap-box it and pass
// the box as user_data with a cdecl trampoline. The DLL fires the callback on
// its own UI thread; marshal to the wx main thread before running the wx-heavy
// completion (wxFileSelector etc.).
struct CompletionBox {
    std::function<void(std::optional<std::string>)> fn;
};

void KLM_CALL completionTrampoline(const char* xmodelPath, void* user) {
    CompletionBox* box = static_cast<CompletionBox*>(user);
    std::optional<std::string> result;
    if (xmodelPath && *xmodelPath) result = std::string(xmodelPath);
    auto fn = std::move(box->fn);
    delete box;
    if (wxTheApp) {
        wxTheApp->CallAfter([fn, result]() { fn(result); });
    } else {
        fn(result);
    }
}

} // namespace

void PresentScanWindow(const std::string& cameraID,
                       const std::string& scanDumpParent,
                       std::function<void(std::optional<std::string>)> completion) {
    auto* box = new CompletionBox{std::move(completion)};
    klm_present_scan_window(cameraID.c_str(), scanDumpParent.c_str(),
                            completionTrampoline, box);
}

void PresentRTSPScanWindow(const std::string& rtspURL,
                           const std::string& username,
                           const std::string& password,
                           const std::string& scanDumpParent,
                           std::function<void(std::optional<std::string>)> completion) {
    auto* box = new CompletionBox{std::move(completion)};
    klm_present_scan_window_rtsp(
        rtspURL.c_str(),
        username.empty() ? nullptr : username.c_str(),
        password.empty() ? nullptr : password.c_str(),
        scanDumpParent.c_str(), completionTrampoline, box);
}

} // namespace klbridge

#endif // _WIN32 || __linux__
