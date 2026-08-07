#import "XLLaunchTiming.h"

#include <chrono>
#include <mutex>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

namespace {

struct Mark {
    std::string name;
    double sinceBeginMs;
    double sincePrevMs;
    bool postLaunch;
};

std::mutex sMutex;
std::chrono::steady_clock::time_point sBegin;
std::chrono::steady_clock::time_point sPrev;
bool sStarted = false;
std::vector<Mark> sMarks;

void RecordMark(NSString* name, bool postLaunch) {
    std::lock_guard<std::mutex> lock(sMutex);
    if (!sStarted) {
        // A mark before begin() would divide by an unset origin; treat the
        // first mark as the origin rather than reporting garbage.
        sBegin = std::chrono::steady_clock::now();
        sPrev = sBegin;
        sStarted = true;
    }
    const auto now = std::chrono::steady_clock::now();
    const double sinceBegin = std::chrono::duration<double, std::milli>(now - sBegin).count();
    const double sincePrev = std::chrono::duration<double, std::milli>(now - sPrev).count();
    sPrev = now;

    const std::string n = name.UTF8String ? name.UTF8String : "?";
    sMarks.push_back({ n, sinceBegin, sincePrev, postLaunch });
    spdlog::info("LAUNCH-PHASE {} {} total={:.1f}ms delta={:.1f}ms",
                 postLaunch ? "post" : "boot", n, sinceBegin, sincePrev);
}

} // namespace

@implementation XLLaunchTiming

+ (void)begin {
    {
        std::lock_guard<std::mutex> lock(sMutex);
        if (sStarted) return;
        sBegin = std::chrono::steady_clock::now();
        sPrev = sBegin;
        sStarted = true;
    }
    // iOS hands us a slow launch's MXAppLaunchDiagnostic on the NEXT launch,
    // and that is the launch whose upload carries it — so this launch's file
    // would describe the wrong run. Keep the previous one alongside; the .prev
    // file is the one that pairs with the payload being uploaded.
    NSString* libraryPath = NSSearchPathForDirectoriesInDomains(
        NSLibraryDirectory, NSUserDomainMask, YES).firstObject;
    if (libraryPath.length == 0) return;
    NSString* logsDir = [libraryPath stringByAppendingPathComponent:@"Logs"];
    NSString* cur = [logsDir stringByAppendingPathComponent:@"xlLaunchTiming.txt"];
    NSString* prev = [logsDir stringByAppendingPathComponent:@"xlLaunchTiming.prev.txt"];
    NSFileManager* fm = [NSFileManager defaultManager];
    if ([fm fileExistsAtPath:cur]) {
        [fm removeItemAtPath:prev error:nil];
        [fm moveItemAtPath:cur toPath:prev error:nil];
    }
}

+ (void)mark:(NSString*)name {
    RecordMark(name, /*postLaunch=*/false);
}

+ (void)markPostLaunch:(NSString*)name {
    RecordMark(name, /*postLaunch=*/true);
}

+ (void)flushSidecar {
    std::vector<Mark> snapshot;
    {
        std::lock_guard<std::mutex> lock(sMutex);
        snapshot = sMarks;
    }
    if (snapshot.empty()) return;

    NSMutableString* out = [NSMutableString string];
    [out appendString:@"# xLights iPad launch phase timing\n"];
    [out appendString:@"# 'boot' phases run before the first frame (what MXAppLaunchDiagnostic measures).\n"];
    [out appendString:@"# 'post' phases run after it, on the way to a usable app.\n"];
    [out appendString:@"# kind  total_ms  delta_ms  phase\n"];
    for (const auto& m : snapshot) {
        [out appendFormat:@"%-5s %9.1f %9.1f  %s\n",
                          m.postLaunch ? "post" : "boot",
                          m.sinceBeginMs, m.sincePrevMs, m.name.c_str()];
    }

    NSString* libraryPath = NSSearchPathForDirectoriesInDomains(
        NSLibraryDirectory, NSUserDomainMask, YES).firstObject;
    if (libraryPath.length == 0) return;
    NSString* logsDir = [libraryPath stringByAppendingPathComponent:@"Logs"];
    [[NSFileManager defaultManager] createDirectoryAtPath:logsDir
                             withIntermediateDirectories:YES
                                              attributes:nil
                                                   error:nil];
    [out writeToFile:[logsDir stringByAppendingPathComponent:@"xlLaunchTiming.txt"]
          atomically:YES
            encoding:NSUTF8StringEncoding
               error:nil];
}

@end
