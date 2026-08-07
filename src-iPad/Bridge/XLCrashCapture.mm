/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLCrashCapture.h"

#include <string>

#include <log.h>

#include "utils/AppCallbacks.h"
#include "utils/xlCrashCapture.h"
#include "utils/xlExceptionDescribe.h"

namespace {
// Both live directly in Library/Logs so the packager's existing "starts with
// xLights" log filter sweeps them into the zip with no extra wiring.
NSString* const kLiveRecordName = @"xLightsCrash.txt";
NSString* const kPendingRecordName = @"xLightsCrash.prev.txt";
} // namespace

@implementation XLCrashCapture

+ (NSString*)pendingRecordFileName
{
    return kPendingRecordName;
}

+ (void)installWithLogsDirectory:(NSString*)logsDir
{
    NSDictionary* info = [[NSBundle mainBundle] infoDictionary];
    NSString* version = info[@"CFBundleShortVersionString"] ?: @"?";
    NSString* build = info[@"CFBundleVersion"] ?: @"?";

    NSString* recordPath = [logsDir stringByAppendingPathComponent:kLiveRecordName];

    xlCrashCapture::InstallSignalHandlers(
        std::string(recordPath.UTF8String),
        std::string(version.UTF8String),
        std::string(build.UTF8String));

    // Worker threads catch and describe their own exceptions but then called the
    // core default, which is std::terminate - so a render-thread throw took the
    // process down with nothing recorded beyond the log line. Route it through
    // the same describe-and-abort the desktop uses; the signal handler above
    // turns the abort into a record with a backtrace.
    AppCallbacks::SetHandleUnhandledException([] {
        spdlog::critical("Unhandled exception on a background thread: {}",
                         DescribeCurrentException());
        spdlog::default_logger()->flush();
        std::abort();
    });

    spdlog::info("Crash capture installed, record path {}", recordPath.UTF8String);
}

+ (BOOL)hasPendingRecord
{
    NSString* libraryPath = NSSearchPathForDirectoriesInDomains(
        NSLibraryDirectory, NSUserDomainMask, YES).firstObject;
    NSString* pending = [[libraryPath stringByAppendingPathComponent:@"Logs"]
        stringByAppendingPathComponent:kPendingRecordName];
    return [[NSFileManager defaultManager] fileExistsAtPath:pending];
}

+ (BOOL)rotatePendingRecord
{
    NSFileManager* fm = [NSFileManager defaultManager];
    NSString* libraryPath = NSSearchPathForDirectoriesInDomains(
        NSLibraryDirectory, NSUserDomainMask, YES).firstObject;
    NSString* logsDir = [libraryPath stringByAppendingPathComponent:@"Logs"];

    NSString* live = [logsDir stringByAppendingPathComponent:kLiveRecordName];
    if (![fm fileExistsAtPath:live]) {
        return NO;
    }

    NSString* pending = [logsDir stringByAppendingPathComponent:kPendingRecordName];
    [fm removeItemAtPath:pending error:nil];
    NSError* err = nil;
    if (![fm moveItemAtPath:live toPath:pending error:&err]) {
        // Leave the live file alone rather than lose it; the next launch tries
        // again, and the handler truncates on write so it cannot grow unbounded.
        spdlog::warn("Could not rotate the crash record: {}",
                     err.localizedDescription.UTF8String);
        return NO;
    }

    spdlog::critical("Previous run left a crash record - it will be uploaded with the next diagnostic bundle.");
    return YES;
}

@end
