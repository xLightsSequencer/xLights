// ObjC++ bridge from xLights C++ side into KLightMapper's @objc API.
// Compiled only on macOS (the whole src-apple-core sync group is
// macOS-only target memberships).

#import "KLightMapperBridge.h"
#import <KLightMapper/KLightMapper-Swift.h>

namespace klbridge {

std::vector<CameraInfo> DiscoverContinuityCameras() {
    std::vector<CameraInfo> out;
    NSArray<KLMCameraInfo*>* cams = [KLM discoverContinuityCameras];
    out.reserve(cams.count);
    for (KLMCameraInfo* c in cams) {
        out.push_back(CameraInfo{
            .identifier         = c.identifier.UTF8String   ? std::string(c.identifier.UTF8String)   : std::string(),
            .localizedName      = c.localizedName.UTF8String ? std::string(c.localizedName.UTF8String) : std::string(),
            .isContinuityCamera = static_cast<bool>(c.isContinuityCamera)
        });
    }
    return out;
}

void PresentScanWindow(const std::string& cameraID,
                       const std::string& scanDumpParent,
                       std::function<void(std::optional<std::string>)> completion) {
    NSString* nsID = [NSString stringWithUTF8String:cameraID.c_str()];
    NSString* nsDump = scanDumpParent.empty()
        ? nil
        : [NSString stringWithUTF8String:scanDumpParent.c_str()];
    // Capture-by-value into the block so the std::function stays
    // alive past PresentScanWindow's return.
    auto cb = std::move(completion);
    [KLM presentScanWindowWithCameraID:nsID
                        scanDumpParent:nsDump
                            completion:^(NSURL* _Nullable url) {
        if (!cb) { return; }
        if (url == nil) {
            cb(std::nullopt);
        } else {
            // -fileSystemRepresentation gives a stable, FS-canonical
            // C string we can copy into std::string.
            const char* path = url.fileURL ? url.fileSystemRepresentation
                                           : url.absoluteString.UTF8String;
            cb(path ? std::optional<std::string>(path) : std::nullopt);
        }
    }];
}

static std::string cppstr(NSString* s) {
    return (s && s.UTF8String) ? std::string(s.UTF8String) : std::string();
}

std::vector<NetworkCamera> DiscoverNetworkCameras(int timeoutMs) {
    std::vector<NetworkCamera> out;
    NSArray<KLMNetworkCamera*>* cams = [KLM discoverNetworkCamerasWithTimeoutMs:timeoutMs];
    out.reserve(cams.count);
    for (KLMNetworkCamera* c in cams) {
        out.push_back(NetworkCamera{
            .name          = cppstr(c.name),
            .host          = cppstr(c.host),
            .deviceService = cppstr(c.deviceService)
        });
    }
    return out;
}

std::vector<CameraProfile> EnumerateCameraProfiles(const std::string& deviceService,
                                                   const std::string& user,
                                                   const std::string& pass) {
    std::vector<CameraProfile> out;
    NSString* ds = [NSString stringWithUTF8String:deviceService.c_str()];
    NSString* u = user.empty() ? nil : [NSString stringWithUTF8String:user.c_str()];
    NSString* p = pass.empty() ? nil : [NSString stringWithUTF8String:pass.c_str()];
    NSArray<KLMCameraProfile*>* profs =
        [KLM enumerateCameraProfilesWithDeviceService:ds username:u password:p];
    out.reserve(profs.count);
    for (KLMCameraProfile* pr in profs) {
        out.push_back(CameraProfile{
            .name    = cppstr(pr.name),
            .rtspUrl = cppstr(pr.rtspURL),
            .width   = (int)pr.width,
            .height  = (int)pr.height,
            .fps     = (int)pr.fps
        });
    }
    return out;
}

void PresentRTSPScanWindow(const std::string& rtspURL,
                           const std::string& username,
                           const std::string& password,
                           const std::string& scanDumpParent,
                           std::function<void(std::optional<std::string>)> completion) {
    NSString* nsURL = [NSString stringWithUTF8String:rtspURL.c_str()];
    NSString* nsUser = username.empty()
        ? nil
        : [NSString stringWithUTF8String:username.c_str()];
    NSString* nsPass = password.empty()
        ? nil
        : [NSString stringWithUTF8String:password.c_str()];
    NSString* nsDump = scanDumpParent.empty()
        ? nil
        : [NSString stringWithUTF8String:scanDumpParent.c_str()];
    // Capture-by-value into the block so the std::function stays
    // alive past PresentRTSPScanWindow's return.
    auto cb = std::move(completion);
    [KLM presentScanWindowWithRtspURL:nsURL
                             username:nsUser
                             password:nsPass
                       scanDumpParent:nsDump
                           completion:^(NSURL* _Nullable url) {
        if (!cb) { return; }
        if (url == nil) {
            cb(std::nullopt);
        } else {
            const char* path = url.fileURL ? url.fileSystemRepresentation
                                           : url.absoluteString.UTF8String;
            cb(path ? std::optional<std::string>(path) : std::nullopt);
        }
    }];
}

} // namespace klbridge
