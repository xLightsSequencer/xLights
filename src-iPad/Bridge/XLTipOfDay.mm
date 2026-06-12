/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLTipOfDay.h"

#include "utils/CachedFileDownloader.h"
#include "utils/ExternalHooks.h"
#include "utils/string_utils.h"

#include <pugixml.hpp>

#include <fstream>
#include <sstream>
#include <string>

// Same TipOfDay content the desktop dialog reads. iPad uses the
// raw.githubusercontent path; tips marked exclude="OSX" are skipped
// (Apple platforms share that desktop exclusion).
static const std::string kTODBase =
    "https://raw.githubusercontent.com/xLightsSequencer/xLights/master/TipOfDay/";

static NSString* NSS(const std::string& s) {
    return [NSString stringWithUTF8String:s.c_str()];
}

@implementation XLTipOfDay

+ (NSString*)baseURL { return NSS(kTODBase); }

+ (void)loadTipsWithCompletion:(void (^)(NSArray<NSDictionary*>* tips))completion {
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        NSMutableArray<NSDictionary*>* tips = [NSMutableArray array];
        const std::string fname = CachedFileDownloader::GetDefaultCache().GetFile(
            kTODBase + "tod.xml", CACHETIME_DAY);
        if (!fname.empty() && FileExists(fname)) {
            pugi::xml_document doc;
            if (doc.load_file(fname.c_str()) && doc.document_element()) {
                for (pugi::xml_node n = doc.document_element().first_child(); n; n = n.next_sibling()) {
                    if (std::string_view(n.name()) != "tip") continue;
                    const std::string url = n.attribute("url").as_string();
                    if (url.empty()) continue;
                    const std::string exclude = n.attribute("exclude").as_string();
                    if (::Contains(::Lower(exclude), "osx")) continue;
                    [tips addObject:@{
                        @"title":    NSS(n.attribute("title").as_string()),
                        @"url":      NSS(url),
                        @"category": NSS(n.attribute("category").as_string()),
                        @"level":    NSS(n.attribute("level").as_string("Beginner")),
                    }];
                }
            }
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            if (completion) completion(tips);
        });
    });
}

+ (void)loadTipHTML:(NSString*)relativeURL
         completion:(void (^)(NSString* _Nullable html))completion {
    if (relativeURL.length == 0) {
        if (completion) completion(nil);
        return;
    }
    const std::string rel = relativeURL.UTF8String;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        const std::string fname = CachedFileDownloader::GetDefaultCache().GetFile(
            kTODBase + rel, CACHETIME_DAY, "html");
        NSString* html = nil;
        if (!fname.empty() && FileExists(fname)) {
            std::ifstream f(fname);
            if (f) {
                std::stringstream ss;
                ss << f.rdbuf();
                html = NSS(ss.str());
            }
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            if (completion) completion(html);
        });
    });
}

@end
