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

#import <Metal/Metal.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include "graphics/GLContextManager.h"
#include "osxUtils/MetalDeviceManager.h"
#include "utils/FileUtils.h"

// Forward declaration — implemented in CoreGraphicsTextDrawingContext.mm
void RegisterCoreGraphicsTextDrawingContext();

#include <string>

static bool sInitialized = false;

@implementation XLiPadInit

+ (void)initialize {
    if (sInitialized) return;
    sInitialized = true;

    // Set up log file in the app's Documents directory
    NSString* docsPath = NSSearchPathForDirectoriesInDomains(
        NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    std::string logPath = std::string([docsPath UTF8String]) + "/xLights.log";

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
    } catch (const spdlog::spdlog_ex& ex) {
        NSLog(@"spdlog init failed: %s", ex.what());
    }

    // Register CoreGraphics-based TextDrawingContext for text/shape effects
    RegisterCoreGraphicsTextDrawingContext();

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
}

@end
