/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLiPadInit.h"
#import "XLAIServices.h"

#import <Metal/Metal.h>
#import <ImageIO/ImageIO.h>
#import <CoreGraphics/CoreGraphics.h>
#import <UIKit/UIKit.h>
#import <sys/utsname.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include "graphics/GLContextManager.h"
#include "osxUtils/MetalDeviceManager.h"
#include "osxUtils/XLMetricKit.h"
#include "render/SequenceMedia.h"
#include "utils/FileUtils.h"
#include "utils/xlImage.h"
#include "xLightsVersion.h"

// Forward declaration — implemented in CoreGraphicsTextDrawingContext.mm
void RegisterCoreGraphicsTextDrawingContext();

#include <string>

static bool sInitialized = false;

namespace {

/// Decode an animated image (GIF or WebP) from raw bytes using ImageIO
/// and populate an `AnimatedImageData` struct. ImageIO on iOS composites
/// GIF frames correctly (disposal + transparency), so every returned
/// frame is already a full display-ready bitmap — we just need to copy
/// the pixels out and stash per-frame delays.
///
/// Both `frames` and `framesNoBG` are populated with the composited
/// pixel data. Pixels that were transparent in the source keep alpha
/// zero in `framesNoBG`; in `frames`, alpha is forced to 255 on every
/// pixel so the "BG visible" path shows a solid plate rather than
/// holes. This preserves the SequenceMedia API semantics (`suppress
/// GIFBackground` still picks between the two) without requiring us to
/// pull the GIF global-palette background color out of ImageIO, which
/// iOS doesn't surface cleanly.
static AnimatedImageData DecodeAnimatedImageIO(const uint8_t* data,
                                                size_t len,
                                                CFStringRef frameDictKey,
                                                CFStringRef delayKey) {
    AnimatedImageData result;
    if (!data || len == 0) return result;

    CFDataRef cfData = CFDataCreate(nullptr, data, (CFIndex)len);
    if (!cfData) return result;

    CGImageSourceRef src = CGImageSourceCreateWithData(cfData, nullptr);
    CFRelease(cfData);
    if (!src) return result;

    size_t frameCount = CGImageSourceGetCount(src);
    if (frameCount == 0) {
        CFRelease(src);
        return result;
    }

    int maxW = 0, maxH = 0;
    result.frames.reserve(frameCount);
    result.framesNoBG.reserve(frameCount);
    result.frameTimes.reserve(frameCount);

    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();

    for (size_t i = 0; i < frameCount; i++) {
        CGImageRef frame = CGImageSourceCreateImageAtIndex(src, i, nullptr);
        if (!frame) continue;

        int w = (int)CGImageGetWidth(frame);
        int h = (int)CGImageGetHeight(frame);
        maxW = std::max(maxW, w);
        maxH = std::max(maxH, h);

        // Render into an RGBA byte buffer. Use kCGImageAlphaPremultipliedLast
        // so the output matches xlImage's RGBA-interleaved layout directly.
        std::unique_ptr<uint8_t[]> rgba(new uint8_t[(size_t)w * h * 4]());
        CGContextRef ctx = CGBitmapContextCreate(rgba.get(), w, h, 8, w * 4, cs,
                                                   (uint32_t)kCGImageAlphaPremultipliedLast
                                                   | kCGBitmapByteOrder32Big);
        if (ctx) {
            CGContextSetBlendMode(ctx, kCGBlendModeCopy);
            CGContextDrawImage(ctx, CGRectMake(0, 0, w, h), frame);
            CGContextRelease(ctx);
        }
        CGImageRelease(frame);

        // framesNoBG keeps alpha so transparent pixels stay transparent.
        xlImage noBG(w, h);
        std::memcpy(noBG.GetData(), rgba.get(), (size_t)w * h * 4);

        // frames forces alpha to 255 so transparent pixels render as
        // whatever RGB the decoder chose for them (typically black) —
        // desktop's "with BG color" variant is closer to this for most
        // non-transparent-intent GIFs.
        for (int p = 0; p < w * h; p++) {
            rgba[p * 4 + 3] = 255;
        }
        xlImage withBG(w, h);
        std::memcpy(withBG.GetData(), rgba.get(), (size_t)w * h * 4);

        result.frames.push_back(std::move(withBG));
        result.framesNoBG.push_back(std::move(noBG));

        // Delay time for this frame in milliseconds. ImageIO reports
        // seconds as a CFNumber in the format-specific sub-dictionary.
        long delayMS = 100;
        CFDictionaryRef props = CGImageSourceCopyPropertiesAtIndex(src, i, nullptr);
        if (props) {
            CFDictionaryRef fd = (CFDictionaryRef)CFDictionaryGetValue(props, frameDictKey);
            if (fd) {
                CFNumberRef delay = (CFNumberRef)CFDictionaryGetValue(fd, delayKey);
                double seconds = 0;
                if (delay && CFNumberGetValue(delay, kCFNumberDoubleType, &seconds)) {
                    delayMS = (long)(seconds * 1000.0);
                    if (delayMS <= 0) delayMS = 100;
                }
            }
            CFRelease(props);
        }
        result.frameTimes.push_back(delayMS);
    }

    CGColorSpaceRelease(cs);
    CFRelease(src);

    result.width = maxW;
    result.height = maxH;
    result.backgroundColor = xlBLACK;
    return result;
}

static void LogMachineConfig() {
    NSDictionary* info = [[NSBundle mainBundle] infoDictionary];
    NSString* shortVer = info[@"CFBundleShortVersionString"] ?: @"?";
    NSString* buildVer = info[@"CFBundleVersion"] ?: @"?";
    spdlog::info("Version: {} (build {})", [shortVer UTF8String], [buildVer UTF8String]);
    spdlog::info("Build Date: {}", xlights_build_date);

    UIDevice* dev = [UIDevice currentDevice];
    struct utsname u;
    uname(&u);
    spdlog::info("Machine configuration:");
    spdlog::info("  OS: {} {}", [dev.systemName UTF8String], [dev.systemVersion UTF8String]);
    spdlog::info("  Device model: {}", u.machine);
    spdlog::info("  Device name: {}", [dev.name UTF8String]);
    spdlog::info("  CPU Arch: {}", u.machine);

    NSProcessInfo* proc = [NSProcessInfo processInfo];
    long long memBytes = (long long)proc.physicalMemory;
    spdlog::info("  Total memory: {} MB", memBytes / (1024 * 1024));
    spdlog::info("  Active processors: {}", (unsigned long)proc.activeProcessorCount);
    spdlog::info("  Thermal state: {}", (long)proc.thermalState);

    NSError* err = nil;
    NSDictionary* fsAttrs = [[NSFileManager defaultManager]
        attributesOfFileSystemForPath:NSHomeDirectory() error:&err];
    if (!err && fsAttrs) {
        long long freeBytes = [fsAttrs[NSFileSystemFreeSize] longLongValue];
        spdlog::info("  Free disk: {} MB", freeBytes / (1024 * 1024));
    }
    spdlog::info("  Locale: {}", [[NSLocale currentLocale].localeIdentifier UTF8String]);
}

} // namespace

@implementation XLiPadInit

+ (void)initialize {
    if (sInitialized) return;
    sInitialized = true;

    // Logs live in <sandbox>/Library/Logs/ — same relative path the
    // sandboxed desktop app uses (~/Library/Containers/.../Library/Logs).
    // Library/Logs is excluded from iCloud backup and not visible to the
    // user via the Files app, both of which Documents/ would have wrong.
    NSFileManager* fm = [NSFileManager defaultManager];
    NSString* libraryPath = NSSearchPathForDirectoriesInDomains(
        NSLibraryDirectory, NSUserDomainMask, YES).firstObject;
    NSString* logsDir = [libraryPath stringByAppendingPathComponent:@"Logs"];
    [fm createDirectoryAtPath:logsDir
        withIntermediateDirectories:YES
                         attributes:nil
                              error:nil];

    // One-time migration: pre-2026.05 builds put xLights.log{,.1,.2}
    // in Documents/. Move any survivors into Library/Logs/ so the new
    // Package Logs flow picks them up.
    NSString* docsPath = NSSearchPathForDirectoriesInDomains(
        NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    for (NSString* name in @[@"xLights.log", @"xLights.1.log", @"xLights.2.log"]) {
        NSString* legacy = [docsPath stringByAppendingPathComponent:name];
        if ([fm fileExistsAtPath:legacy]) {
            NSString* dest = [logsDir stringByAppendingPathComponent:name];
            [fm removeItemAtPath:dest error:nil];
            [fm moveItemAtPath:legacy toPath:dest error:nil];
        }
    }

    std::string logPath = std::string([logsDir UTF8String]) + "/xLights.log";

    try {
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath, 1024 * 1024 * 5, 3);
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        // Create default logger with both sinks
        std::vector<spdlog::sink_ptr> sinks = {file_sink, console_sink};
        auto default_logger = std::make_shared<spdlog::logger>("xLights", sinks.begin(), sinks.end());
        spdlog::initialize_logger(default_logger);
        spdlog::set_default_logger(default_logger);
        spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e [%n %l] %v");
        spdlog::flush_on(spdlog::level::info);

        // Create named loggers that core code expects
        auto render_logger = std::make_shared<spdlog::logger>("render", sinks.begin(), sinks.end());
        auto curl_logger = std::make_shared<spdlog::logger>("curl", sinks.begin(), sinks.end());
        auto job_logger = std::make_shared<spdlog::logger>("job", sinks.begin(), sinks.end());
        auto work_logger = std::make_shared<spdlog::logger>("work", sinks.begin(), sinks.end());

        render_logger->set_level(spdlog::level::warn);
        curl_logger->set_level(spdlog::level::info);
        job_logger->set_level(spdlog::level::info);
        work_logger->set_level(spdlog::level::info);

        spdlog::register_logger(render_logger);
        spdlog::register_logger(curl_logger);
        spdlog::register_logger(job_logger);
        spdlog::register_logger(work_logger);

        spdlog::info("xLights iPad initialized, log at {}", logPath);
        LogMachineConfig();
    } catch (const spdlog::spdlog_ex& ex) {
        NSLog(@"spdlog init failed: %s", ex.what());
    }

    // MetricKit collector — JSON payloads land alongside the log files,
    // and Tools > Package Logs sweeps them into the user-shared zip.
    std::string diagnosticsDir = std::string([logsDir UTF8String]) + "/Diagnostics";
    StartMetricKitCollection(diagnosticsDir);

    // Register CoreGraphics-based TextDrawingContext for text/shape effects
    RegisterCoreGraphicsTextDrawingContext();

    // GIF loading is handled by the core stb_image loader (LoadAnimatedGIFFromMemory) — no registration needed.
    // Register WebP animation loader using iOS ImageIO.
    // ImageIO exposes WebP via kCGImagePropertyWebPDictionary on
    // sufficiently new SDKs; fall back to the GIF dict's delay key
    // since the ImageIO WebP-specific key isn't universally available.
    ImageCacheEntry::SetWebPLoader([](const uint8_t* data, size_t len,
                                       const std::string& /*filename*/) -> AnimatedImageData {
        return DecodeAnimatedImageIO(data, len,
                                      kCGImagePropertyWebPDictionary,
                                      kCGImagePropertyWebPDelayTime);
    });

    // Set resources directory to app bundle path (where controllers/, images/ etc. live)
    NSString* resourcesPath = [[NSBundle mainBundle] resourcePath];
    FileUtils::SetResourcesDir(std::string([resourcesPath UTF8String]));
    spdlog::info("Resources dir: {}", FileUtils::GetResourcesDir());

    // Initialize MetalDeviceManager (needed for Metal compute effects and ANGLE GPU matching)
    MetalDeviceManager::instance().retain();

    // Initialize GLContextManager with ANGLE (Metal backend)
    GLContextManager::InitParams glParams;
    glParams.metalDeviceRegistryID = MetalDeviceManager::instance().getMTLDevice().registryID;
    GLContextManager::Instance().Initialize(glParams);

    spdlog::info("GLContextManager initialized with ANGLE Metal backend, device registry ID: {}",
                 glParams.metalDeviceRegistryID);

    // Construct the AI ServiceManager + iPad settings store. Built-in
    // services (chatGPT, claude, ollama, gemini, GenericClient) are
    // registered immediately and load any persisted settings/secrets
    // from NSUserDefaults + Keychain. AI plugin loading is intentionally
    // skipped on iOS — App Store policy forbids dynamic libraries.
    (void)[XLAIServices shared];
}

@end
