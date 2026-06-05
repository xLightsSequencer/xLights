/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Phase J-4 (model download) — Swift-facing facade over the
// shared `vendor_catalog::Catalog` in src-core. Fetches the
// xlights.org vendor index + each vendor's inventory via the
// `CachedFileDownloader`, then serializes the result into
// NSArray<NSDictionary*> form so SwiftUI can read it without
// holding C++ refs.
//
// Vendor / model / wiring dictionaries are stable shapes — see
// XLVendorCatalog.mm for the exact keys (camelCase NSStrings
// matching the catalog field names).

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface XLVendorCatalog : NSObject

// Async load of the master vendor index + every vendor's
// inventory. Progress is dispatched to the main queue on each
// vendor step; completion runs on the main queue once the
// catalog is parsed (or failed). On success `vendors` is
// populated and `errorMessage` is nil; on failure `vendors` is
// empty and `errorMessage` carries the first parse / network
// failure encountered.
- (void)loadWithProgress:(void (^)(int percent, NSString* label))progress
              completion:(void (^)(NSString* _Nullable errorMessage))completion;

// Serialized vendor records. Each entry contains:
//   name, contact, email, phone, website, facebook, twitter,
//   notes, logoFile (paths), maxModels (NSNumber), models
// `models` is an NSArray of NSDictionary with keys:
//   id, name, type, material, thickness, width, height, depth,
//   pixelCount, pixelSpacing, pixelDescription, webpage, notes,
//   imageFiles (NSArray<NSString*>), wirings (NSArray)
// Each wiring contains:
//   name, wiringDescription, xmodelLink, imageFiles,
//   modelWidthMM, modelHeightMM, modelDepthMM (NSNumbers).
@property (nonatomic, strong, readonly) NSArray<NSDictionary*>* vendors;

// Download a wiring's `.xmodel` (or `.zip`) into the shared
// file cache. Async — completion fires on the main queue with
// the local path on success, nil + error message on failure.
// `pixelDescription` / `pixelSpacing` / `pixelCount` patch the
// downloaded xmodel's pixel-attribute metadata (the wiring
// itself has these fields on the parent model, so the catalog
// dictionary carries them in its model entry).
- (void)downloadWiringXmodelFromURL:(NSString*)xmodelLink
                   pixelDescription:(nullable NSString*)pixelDescription
                       pixelSpacing:(nullable NSString*)pixelSpacing
                         pixelCount:(nullable NSString*)pixelCount
                            widthMM:(NSInteger)widthMM
                           heightMM:(NSInteger)heightMM
                            depthMM:(NSInteger)depthMM
                         completion:(void (^)(NSString* _Nullable localPath,
                                              NSString* _Nullable errorMessage))completion;

// Async image fetch via the shared `CachedFileDownloader`. Hits
// the cache first (`CACHETIME_LONG`); falls through to a network
// download if the file is missing or stale. Completion runs on
// the main queue with the local file path or nil. Designed for
// SwiftUI thumbnails — vendor catalogs list dozens of image URLs
// per model, so eager downloads on catalog load would be wasteful.
- (void)fetchImageFromURL:(NSString*)imageURL
               completion:(void (^)(NSString* _Nullable localPath))completion;

@end

NS_ASSUME_NONNULL_END
