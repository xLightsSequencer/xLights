/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Swift-facing facade over the shared `music_catalog::Catalog` in
// src-core — the sequences/lyrics side of the vendor browser
// (desktop's `VendorMusicDialog`). Mirrors `XLVendorCatalog` for
// the models side: async load → NSArray<NSDictionary*> snapshot,
// then a download op that lands the file straight in the show
// folder rather than the shared cache.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface XLMusicCatalog : NSObject

// Async load of the master vendor index + every music vendor's
// inventory. Progress on the main queue per vendor step;
// completion on the main queue with nil on success or an error
// message on failure.
- (void)loadWithProgress:(void (^)(int percent, NSString* label))progress
              completion:(void (^)(NSString* _Nullable errorMessage))completion;

// Serialized vendor records. Each entry contains:
//   name, contact, email, phone, website, facebook, twitter,
//   notes, logoFile, items
// `items` is an NSArray of NSDictionary with keys:
//   title, artist, creator, notes, type ("Sequence"/"Lyrics"),
//   typeName, webpage, download, video, music, ext, fileName
@property (nonatomic, strong, readonly) NSArray<NSDictionary*>* vendors;

// Download a sequence/lyric `downloadURL` into `destFolder`
// (typically the show folder) under `fileName`. Async; completion
// on the main queue with the saved local path on success, nil +
// error on failure.
- (void)downloadItemFromURL:(NSString*)downloadURL
                   fileName:(NSString*)fileName
                  destFolder:(NSString*)destFolder
                 completion:(void (^)(NSString* _Nullable localPath,
                                      NSString* _Nullable errorMessage))completion;

@end

NS_ASSUME_NONNULL_END
