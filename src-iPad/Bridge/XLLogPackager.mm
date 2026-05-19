#import "XLLogPackager.h"
#import "XLSequenceDocument.h"

#import <UIKit/UIKit.h>
#import <sys/sysctl.h>
#import <sys/utsname.h>
#import <mach/mach.h>

#include <string>

#include "utils/Parallel.h"

namespace {

NSString* DeviceModelIdentifier() {
    struct utsname systemInfo;
    uname(&systemInfo);
    return [NSString stringWithCString:systemInfo.machine
                              encoding:NSUTF8StringEncoding];
}

NSString* FreeDiskSpaceString() {
    NSError* err = nil;
    NSDictionary* attrs = [[NSFileManager defaultManager]
        attributesOfFileSystemForPath:NSHomeDirectory() error:&err];
    if (err || !attrs) return @"unknown";
    long long free = [attrs[NSFileSystemFreeSize] longLongValue];
    return [NSByteCountFormatter stringFromByteCount:free
                                          countStyle:NSByteCountFormatterCountStyleFile];
}

NSString* PhysicalMemoryString() {
    return [NSByteCountFormatter
        stringFromByteCount:[NSProcessInfo processInfo].physicalMemory
                 countStyle:NSByteCountFormatterCountStyleMemory];
}

void CopyTreeIntoStaging(NSFileManager* fm,
                         NSString* srcDir,
                         NSString* destDir,
                         NSPredicate* _Nullable filter) {
    BOOL isDir = NO;
    if (![fm fileExistsAtPath:srcDir isDirectory:&isDir] || !isDir) return;

    NSDirectoryEnumerator* en = [fm enumeratorAtPath:srcDir];
    NSString* relative;
    while ((relative = [en nextObject])) {
        if (filter && ![filter evaluateWithObject:relative]) continue;
        NSString* src = [srcDir stringByAppendingPathComponent:relative];
        BOOL childIsDir = NO;
        [fm fileExistsAtPath:src isDirectory:&childIsDir];
        if (childIsDir) continue;
        NSString* dst = [destDir stringByAppendingPathComponent:relative];
        [fm createDirectoryAtPath:[dst stringByDeletingLastPathComponent]
      withIntermediateDirectories:YES
                       attributes:nil
                            error:nil];
        [fm copyItemAtPath:src toPath:dst error:nil];
    }
}

NSString* DeviceInfoText() {
    NSDictionary* info = [[NSBundle mainBundle] infoDictionary];
    NSMutableString* s = [NSMutableString string];
    [s appendFormat:@"App version: %@ (build %@)\n",
        info[@"CFBundleShortVersionString"] ?: @"?",
        info[@"CFBundleVersion"] ?: @"?"];
    [s appendFormat:@"OS: %@ %@\n",
        [UIDevice currentDevice].systemName,
        [UIDevice currentDevice].systemVersion];
    [s appendFormat:@"Device model: %@\n", DeviceModelIdentifier()];
    [s appendFormat:@"Device name: %@\n", [UIDevice currentDevice].name];
    [s appendFormat:@"Locale: %@\n", [NSLocale currentLocale].localeIdentifier];
    [s appendFormat:@"Physical memory: %@\n", PhysicalMemoryString()];
    [s appendFormat:@"Free disk: %@\n", FreeDiskSpaceString()];
    [s appendFormat:@"Active processors: %lu\n",
        (unsigned long)[NSProcessInfo processInfo].activeProcessorCount];
    [s appendFormat:@"Thermal state: %ld\n",
        (long)[NSProcessInfo processInfo].thermalState];
    [s appendFormat:@"Captured: %@\n", [NSDate date]];
    return s;
}

NSString* ThreadsText() {
    std::string status = "Parallel Job Pool:\n";
    status += ParallelJobPool::POOL.GetThreadStatus();
    return [NSString stringWithUTF8String:status.c_str()];
}

} // namespace

// Internal helper. `document` may be nil. `includeUserContent` controls
// whether the show folder XML and open sequence are copied in.
// `destDir` is the final directory where the zip should live (caller's
// choice — NSTemporaryDirectory for share-sheet, Library/Logs/PendingUpload
// for auto-upload). Returns the resulting zip URL or nil.
static NSURL* BuildLogZip(XLSequenceDocument* _Nullable document,
                          BOOL includeUserContent,
                          NSURL* destDir,
                          NSString* baseNameStem,
                          NSError** outError) {
    NSFileManager* fm = [NSFileManager defaultManager];

    NSDateFormatter* fmt = [[NSDateFormatter alloc] init];
    fmt.dateFormat = @"yyyyMMdd-HHmmss";
    fmt.timeZone = [NSTimeZone timeZoneWithAbbreviation:@"UTC"];
    NSString* stamp = [fmt stringFromDate:[NSDate date]];
    NSString* baseName = [NSString stringWithFormat:@"%@-%@", baseNameStem, stamp];

    NSURL* tmpDir = [NSURL fileURLWithPath:NSTemporaryDirectory() isDirectory:YES];
    NSURL* stagingDir = [tmpDir URLByAppendingPathComponent:baseName isDirectory:YES];
    [fm removeItemAtURL:stagingDir error:nil];
    if (![fm createDirectoryAtURL:stagingDir
        withIntermediateDirectories:YES
                         attributes:nil
                              error:outError]) {
        return nil;
    }

    NSString* libraryPath = NSSearchPathForDirectoriesInDomains(
        NSLibraryDirectory, NSUserDomainMask, YES).firstObject;
    NSString* logsDir = [libraryPath stringByAppendingPathComponent:@"Logs"];

    // 1. Log files (xLights.log + rotated siblings — but not the
    //    Diagnostics subdirectory, which we copy separately so it
    //    keeps its own folder in the zip).
    NSPredicate* logFilter = [NSPredicate predicateWithBlock:
        ^BOOL(NSString* path, NSDictionary*) {
            if ([path hasPrefix:@"Diagnostics"]) return NO;
            if ([path hasPrefix:@"Sessions"]) return NO;
            return [path.lastPathComponent hasPrefix:@"xLights"];
        }];
    CopyTreeIntoStaging(fm, logsDir, stagingDir.path, logFilter);

    // 2. MetricKit JSON payloads.
    NSString* diagnosticsSrc = [logsDir stringByAppendingPathComponent:@"Diagnostics"];
    NSString* diagnosticsDst = [stagingDir.path stringByAppendingPathComponent:@"Diagnostics"];
    CopyTreeIntoStaging(fm, diagnosticsSrc, diagnosticsDst, nil);

    // 3. Show folder XML + currently-open sequence — full-payload only.
    if (includeUserContent) {
        if (document.showFolderPath.length > 0) {
            NSString* showStaging = [stagingDir.path stringByAppendingPathComponent:@"show"];
            [fm createDirectoryAtPath:showStaging
          withIntermediateDirectories:YES
                           attributes:nil
                                error:nil];
            for (NSString* name in @[@"xlights_networks.xml", @"xlights_rgbeffects.xml"]) {
                NSString* src = [document.showFolderPath stringByAppendingPathComponent:name];
                if ([fm fileExistsAtPath:src]) {
                    [fm copyItemAtPath:src
                                toPath:[showStaging stringByAppendingPathComponent:name]
                                 error:nil];
                }
            }
        }
        if (document.isSequenceLoaded && document.currentSequencePath.length > 0) {
            NSString* seqPath = document.currentSequencePath;
            if ([fm fileExistsAtPath:seqPath]) {
                NSString* dst = [stagingDir.path
                    stringByAppendingPathComponent:seqPath.lastPathComponent];
                [fm copyItemAtPath:seqPath toPath:dst error:nil];
            }
        }
    }

    // 4. Sidecar text files.
    [DeviceInfoText() writeToFile:[stagingDir.path stringByAppendingPathComponent:@"device-info.txt"]
                       atomically:YES
                         encoding:NSUTF8StringEncoding
                            error:nil];
    [ThreadsText() writeToFile:[stagingDir.path stringByAppendingPathComponent:@"threads.txt"]
                    atomically:YES
                      encoding:NSUTF8StringEncoding
                         error:nil];

    // 5. Zip via NSFileCoordinator. .forUploading hands back a
    //    temporary zipped copy of the directory; copy it into the
    //    caller's chosen destination directory.
    [fm createDirectoryAtURL:destDir
   withIntermediateDirectories:YES
                    attributes:nil
                         error:nil];

    __block NSURL* finalZip = nil;
    NSError* coordErr = nil;
    NSFileCoordinator* coord = [[NSFileCoordinator alloc] init];
    [coord coordinateReadingItemAtURL:stagingDir
                              options:NSFileCoordinatorReadingForUploading
                                error:&coordErr
                           byAccessor:^(NSURL* zippedURL) {
        NSURL* dst = [destDir URLByAppendingPathComponent:
            [NSString stringWithFormat:@"%@.zip", baseName]];
        [fm removeItemAtURL:dst error:nil];
        NSError* copyErr = nil;
        if ([fm copyItemAtURL:zippedURL toURL:dst error:&copyErr]) {
            finalZip = dst;
        } else if (outError) {
            *outError = copyErr;
        }
    }];

    [fm removeItemAtURL:stagingDir error:nil];

    if (coordErr) {
        if (outError) *outError = coordErr;
        return nil;
    }
    return finalZip;
}

@implementation XLLogPackager

+ (nullable NSURL*)packageLogsForDocument:(nullable XLSequenceDocument*)document
                                    error:(NSError* _Nullable* _Nullable)outError {
    NSURL* tmpDir = [NSURL fileURLWithPath:NSTemporaryDirectory() isDirectory:YES];
    return BuildLogZip(document,
                       /*includeUserContent=*/YES,
                       tmpDir,
                       @"xLights-logs",
                       outError);
}

+ (nullable NSURL*)stagePendingUploadWithError:(NSError* _Nullable* _Nullable)outError {
    NSString* libraryPath = NSSearchPathForDirectoriesInDomains(
        NSLibraryDirectory, NSUserDomainMask, YES).firstObject;
    NSURL* pendingDir = [NSURL fileURLWithPath:
        [[libraryPath stringByAppendingPathComponent:@"Logs"]
                stringByAppendingPathComponent:@"PendingUpload"]
                                   isDirectory:YES];
    return BuildLogZip(/*document=*/nil,
                       /*includeUserContent=*/NO,
                       pendingDir,
                       @"xLights-diag",
                       outError);
}

@end
