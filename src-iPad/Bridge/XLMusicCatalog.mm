/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLMusicCatalog.h"

#include "import_export/MusicCatalog.h"

#include <memory>
#include <string>

static NSString* NSS(const std::string& s) {
    return [NSString stringWithUTF8String:s.c_str()];
}

@implementation XLMusicCatalog {
    std::unique_ptr<music_catalog::Catalog> _catalog;
    NSArray<NSDictionary*>* _vendors;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _vendors = @[];
    }
    return self;
}

- (NSArray<NSDictionary*>*)vendors { return _vendors; }

- (void)loadWithProgress:(void (^)(int percent, NSString* label))progress
              completion:(void (^)(NSString* _Nullable errorMessage))completion {
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        auto catalog = std::make_unique<music_catalog::Catalog>();
        auto progressFn = [progress](int pct, const std::string& label) -> bool {
            if (progress) {
                NSString* lbl = NSS(label);
                dispatch_async(dispatch_get_main_queue(), ^{
                    progress(pct, lbl);
                });
            }
            return true;
        };
        const bool ok = catalog->Load(progressFn);

        NSMutableArray<NSDictionary*>* vendorArr = [NSMutableArray array];
        for (const auto* v : catalog->Vendors()) {
            NSMutableArray<NSDictionary*>* itemArr = [NSMutableArray array];
            for (const auto* it : v->_items) {
                [itemArr addObject:@{
                    @"title":    NSS(it->_title),
                    @"artist":   NSS(it->_artist),
                    @"creator":  NSS(it->_creator),
                    @"notes":    NSS(it->_notes),
                    @"typeName": NSS(it->GetTypeName()),
                    @"webpage":  NSS(it->_webpage),
                    @"download": NSS(it->_download),
                    @"video":    NSS(it->_video),
                    @"music":    NSS(it->_music),
                    @"ext":      NSS(it->GetExt()),
                    @"fileName": NSS(it->GetDownloadFileName()),
                }];
            }
            [vendorArr addObject:@{
                @"name":     NSS(v->_name),
                @"contact":  NSS(v->_contact),
                @"email":    NSS(v->_email),
                @"phone":    NSS(v->_phone),
                @"website":  NSS(v->_website),
                @"facebook": NSS(v->_facebook),
                @"twitter":  NSS(v->_twitter),
                @"notes":    NSS(v->_notes),
                @"logoFile": NSS(v->_logoFile),
                @"items":    itemArr,
            }];
        }

        music_catalog::Catalog* catalogRaw = catalog.release();
        dispatch_async(dispatch_get_main_queue(), ^{
            self->_catalog.reset(catalogRaw);
            self->_vendors = vendorArr;
            if (completion) {
                completion(ok ? nil : @"Couldn't load the sequence/lyric catalog. Check your network connection and try again.");
            }
        });
    });
}

- (void)downloadItemFromURL:(NSString*)downloadURL
                   fileName:(NSString*)fileName
                  destFolder:(NSString*)destFolder
                 completion:(void (^)(NSString* _Nullable localPath,
                                      NSString* _Nullable errorMessage))completion {
    if (downloadURL.length == 0) {
        if (completion) completion(nil, @"This item has no download link.");
        return;
    }
    if (destFolder.length == 0) {
        if (completion) completion(nil, @"No show folder is loaded.");
        return;
    }
    const std::string url = downloadURL.UTF8String;
    const std::string fname = fileName.length ? std::string(fileName.UTF8String) : std::string();
    const std::string dest = destFolder.UTF8String;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        const std::string saved = music_catalog::Catalog::DownloadTo(url, dest, fname);
        NSString* result = saved.empty() ? nil : NSS(saved);
        NSString* err = saved.empty()
            ? @"Couldn't download the file. Check your network and try again."
            : nil;
        dispatch_async(dispatch_get_main_queue(), ^{
            if (completion) completion(result, err);
        });
    });
}

@end
