#import "XLDiagnosticSession.h"

#import <UIKit/UIKit.h>

namespace {

NSString* SentinelDir() {
    NSString* lib = NSSearchPathForDirectoriesInDomains(
        NSLibraryDirectory, NSUserDomainMask, YES).firstObject;
    NSString* dir = [[lib stringByAppendingPathComponent:@"Logs"]
                          stringByAppendingPathComponent:@"Sessions"];
    [[NSFileManager defaultManager] createDirectoryAtPath:dir
                              withIntermediateDirectories:YES
                                               attributes:nil
                                                    error:nil];
    return dir;
}

NSString* SentinelPath() {
    return [SentinelDir() stringByAppendingPathComponent:@"current-session.txt"];
}

} // namespace

@implementation XLDiagnosticSession

+ (NSDictionary*)readStaleSentinel {
    NSString* path = SentinelPath();
    if (![[NSFileManager defaultManager] fileExistsAtPath:path]) return nil;
    NSData* data = [NSData dataWithContentsOfFile:path];
    if (data.length == 0) return nil;
    id obj = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
    return [obj isKindOfClass:[NSDictionary class]] ? obj : nil;
}

+ (void)clearStaleSentinel {
    [[NSFileManager defaultManager] removeItemAtPath:SentinelPath() error:nil];
}

+ (void)beginCurrentSession {
    NSDictionary* info = @{
        @"sessionUUID": [NSUUID UUID].UUIDString,
        @"startTime": @([NSDate date].timeIntervalSince1970),
        @"appVersion": [NSBundle mainBundle].infoDictionary[@"CFBundleShortVersionString"] ?: @"?",
        @"build": [NSBundle mainBundle].infoDictionary[@"CFBundleVersion"] ?: @"?",
        @"systemVersion": [UIDevice currentDevice].systemVersion,
    };
    NSError* err = nil;
    NSData* json = [NSJSONSerialization dataWithJSONObject:info
                                                   options:NSJSONWritingPrettyPrinted
                                                     error:&err];
    if (json) [json writeToFile:SentinelPath() atomically:YES];
}

+ (void)endCurrentSession {
    [[NSFileManager defaultManager] removeItemAtPath:SentinelPath() error:nil];
}

@end
