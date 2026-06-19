/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLVendorCatalog.h"

#include "import_export/VendorCatalog.h"
#include "utils/CachedFileDownloader.h"
#include "utils/ExternalHooks.h"

#include <memory>
#include <string>

static NSString* NSS(const std::string& s) {
    return [NSString stringWithUTF8String:s.c_str()];
}

static NSArray<NSString*>* NSArrayOf(const std::list<std::string>& src) {
    NSMutableArray<NSString*>* out = [NSMutableArray arrayWithCapacity:src.size()];
    for (const auto& s : src) {
        [out addObject:NSS(s)];
    }
    return out;
}

@implementation XLVendorCatalog {
    std::unique_ptr<vendor_catalog::Catalog> _catalog;
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
    // The catalog load is synchronous (CachedFileDownloader uses
    // libcurl with no async hook), so run it on a global queue
    // and bounce progress + completion back to the main queue.
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        auto catalog = std::make_unique<vendor_catalog::Catalog>();
        auto progressFn = [progress](int pct, const std::string& label) -> bool {
            if (progress) {
                NSString* lbl = NSS(label);
                dispatch_async(dispatch_get_main_queue(), ^{
                    progress(pct, lbl);
                });
            }
            return true;  // never cancel from inside the catalog itself
        };
        const bool ok = catalog->Load(progressFn);

        // Serialise on the worker thread; the result is a value
        // tree of NSArray/NSDictionary/NSString/NSNumber that's
        // safe to hand to the main queue.
        NSMutableArray<NSDictionary*>* vendorArr = [NSMutableArray array];
        for (const auto* v : catalog->Vendors()) {
            NSMutableArray<NSDictionary*>* modelArr = [NSMutableArray array];
            for (const auto* m : v->_models) {
                NSMutableArray<NSDictionary*>* wireArr = [NSMutableArray array];
                for (const auto* w : m->_wiring) {
                    [wireArr addObject:@{
                        @"name":              NSS(w->_name),
                        @"wiringDescription": NSS(w->_wiringDescription),
                        @"xmodelLink":        NSS(w->_xmodelLink),
                        @"imageURLs":         NSArrayOf(w->_images),
                        @"imageFiles":        NSArrayOf(w->_imageFiles),
                        @"modelWidthMM":      @(w->_modelWidthMM),
                        @"modelHeightMM":     @(w->_modelHeightMM),
                        @"modelDepthMM":      @(w->_modelDepthMM),
                    }];
                }
                [modelArr addObject:@{
                    @"id":               NSS(m->_id),
                    @"name":             NSS(m->_name),
                    @"type":             NSS(m->_type),
                    @"material":         NSS(m->_material),
                    @"thickness":        NSS(m->_thickness),
                    @"width":            NSS(m->_width),
                    @"height":           NSS(m->_height),
                    @"depth":            NSS(m->_depth),
                    @"pixelCount":       NSS(m->_pixelCount),
                    @"pixelSpacing":     NSS(m->_pixelSpacing),
                    @"pixelDescription": NSS(m->_pixelDescription),
                    @"webpage":          NSS(m->_webpage),
                    @"notes":            NSS(m->_notes),
                    @"imageURLs":        NSArrayOf(m->_images),
                    @"imageFiles":       NSArrayOf(m->_imageFiles),
                    @"wirings":          wireArr,
                }];
            }
            [vendorArr addObject:@{
                @"name":      NSS(v->_name),
                @"contact":   NSS(v->_contact),
                @"email":     NSS(v->_email),
                @"phone":     NSS(v->_phone),
                @"website":   NSS(v->_website),
                @"facebook":  NSS(v->_facebook),
                @"twitter":   NSS(v->_twitter),
                @"notes":     NSS(v->_notes),
                @"logoFile":  NSS(v->_logoFile),
                @"maxModels": @(v->_maxModels),
                @"models":    modelArr,
            }];
        }

        // Hand the catalog to the main queue via raw pointer —
        // ObjC blocks can't capture unique_ptr by value.
        vendor_catalog::Catalog* catalogRaw = catalog.release();
        dispatch_async(dispatch_get_main_queue(), ^{
            self->_catalog.reset(catalogRaw);
            self->_vendors = vendorArr;
            if (completion) {
                completion(ok ? nil : @"Couldn't load vendor catalog. Check your network connection and try again.");
            }
        });
    });
}

- (void)downloadWiringXmodelFromURL:(NSString*)xmodelLink
                   pixelDescription:(NSString*)pixelDescription
                       pixelSpacing:(NSString*)pixelSpacing
                         pixelCount:(NSString*)pixelCount
                            widthMM:(NSInteger)widthMM
                           heightMM:(NSInteger)heightMM
                            depthMM:(NSInteger)depthMM
                         completion:(void (^)(NSString* _Nullable localPath,
                                              NSString* _Nullable errorMessage))completion {
    if (xmodelLink.length == 0) {
        if (completion) completion(nil, @"Wiring has no download URL.");
        return;
    }
    const std::string url = xmodelLink.UTF8String;
    const std::string pd = pixelDescription ? std::string(pixelDescription.UTF8String) : "";
    const std::string ps = pixelSpacing ? std::string(pixelSpacing.UTF8String) : "";
    const std::string pc = pixelCount ? std::string(pixelCount.UTF8String) : "";
    const int wMM = (int)widthMM;
    const int hMM = (int)heightMM;
    const int dMM = (int)depthMM;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        // Build a transient ModelWiring just to reuse its
        // download + pixel-patching logic. Construct via a tiny
        // synthetic XML so we don't need to expose more bridge.
        std::string ext = "xmodel";
        if (url.size() >= 4) {
            const std::string tail = url.substr(url.size() - 4);
            std::string lower;
            lower.reserve(tail.size());
            for (char c : tail) lower += (char)std::tolower((unsigned char)c);
            if (lower == ".zip") ext = "zip";
        }
        const std::string localPath = CachedFileDownloader::GetDefaultCache().GetFile(
            url, CACHETIME_LONG, ext);

        NSString* result = nil;
        NSString* err = nil;
        if (localPath.empty() || !FileExists(localPath)) {
            err = @"Couldn't download the model file. Check your network and try again.";
        } else {
            // Patch in pixel attrs when the downloaded xmodel
            // lacks them — desktop's MModelWiring::DownloadXModel
            // does the same.
            if (ext == "xmodel") {
                pugi::xml_document d;
                if (d.load_file(localPath.c_str())) {
                    pugi::xml_node root = d.document_element();
                    if (root) {
                        bool changed = false;
                        if (!root.attribute("PixelType") && !pd.empty()) {
                            root.append_attribute("PixelType") = pd.c_str();
                            changed = true;
                        }
                        if (!root.attribute("PixelMinimumSpacingInches")) {
                            long spacing = std::strtol(ps.c_str(), nullptr, 10);
                            if (spacing != 0) {
                                root.append_attribute("PixelMinimumSpacingInches") = (int)spacing;
                                changed = true;
                            }
                        }
                        if (!root.attribute("PixelCount")) {
                            long count = std::strtol(pc.c_str(), nullptr, 10);
                            if (count != 0) {
                                root.append_attribute("PixelCount") = (int)count;
                                changed = true;
                            }
                        }
                        // J-4 (import) — carry the catalog's
                        // real-world dimensions across into the
                        // xmodel as `widthmm` / `heightmm` /
                        // `depthmm` root attributes. The bridge's
                        // importXmodel path picks these up and
                        // calls Model::ApplyDimensions when the
                        // show has a ruler set, so the imported
                        // model lands at its real size in
                        // whatever units the show uses. Catalog
                        // values WIN over anything already in the
                        // file — the catalog is the source of
                        // truth for catalog-distributed models.
                        if (wMM > 0) {
                            if (root.attribute("widthmm"))  root.remove_attribute("widthmm");
                            root.append_attribute("widthmm") = wMM;
                            changed = true;
                        }
                        if (hMM > 0) {
                            if (root.attribute("heightmm")) root.remove_attribute("heightmm");
                            root.append_attribute("heightmm") = hMM;
                            changed = true;
                        }
                        if (dMM > 0) {
                            if (root.attribute("depthmm"))  root.remove_attribute("depthmm");
                            root.append_attribute("depthmm") = dMM;
                            changed = true;
                        }
                        if (changed) d.save_file(localPath.c_str());
                    }
                }
            }
            result = NSS(localPath);
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            if (completion) completion(result, err);
        });
    });
}

- (void)fetchImageFromURL:(NSString*)imageURL
               completion:(void (^)(NSString* _Nullable localPath))completion {
    if (imageURL.length == 0) {
        if (completion) completion(nil);
        return;
    }
    const std::string url = imageURL.UTF8String;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        const std::string path = CachedFileDownloader::GetDefaultCache().GetFile(
            url, CACHETIME_LONG);
        NSString* result = (path.empty() || !FileExists(path)) ? nil : NSS(path);
        dispatch_async(dispatch_get_main_queue(), ^{
            if (completion) completion(result);
        });
    });
}

@end
