/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Swift-facing facade over the shared Tip of the Day content
// (desktop's TipOfTheDayDialog). Fetches the same tod.xml index +
// per-tip HTML from the xLights GitHub TipOfDay folder via the
// shared CachedFileDownloader, skipping tips the platform excludes
// ("OSX" is excluded on iPad too — same desktop-Apple exclusion).

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface XLTipOfDay : NSObject

// The remote base URL for tip HTML, so a sheet can render images /
// links relative to it.
+ (NSString*)baseURL;

// Async: download + parse tod.xml, filtering out platform-excluded
// tips. Completion on the main queue with one NSDictionary per tip
// (keys: title, url, category, level — all NSString). A value tree
// of NSDictionary/NSString is Sendable-friendly for Swift callers.
// Empty array on failure.
+ (void)loadTipsWithCompletion:(void (^)(NSArray<NSDictionary*>* tips))completion
    NS_SWIFT_NAME(loadTips(completion:));

// Async: fetch a tip's HTML body (the `url` of an entry). Completion
// on the main queue with the HTML string, or nil on failure.
+ (void)loadTipHTML:(NSString*)relativeURL
         completion:(void (^)(NSString* _Nullable html))completion
    NS_SWIFT_NAME(loadTipHTML(_:completion:));

@end

NS_ASSUME_NONNULL_END
