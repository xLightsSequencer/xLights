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

} // namespace klbridge
