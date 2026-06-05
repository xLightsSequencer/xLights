#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import <Foundation/Foundation.h>

@class XLSequenceDocument;

NS_ASSUME_NONNULL_BEGIN

// Image-resize choice. Values map 1:1 onto SuperStar::ImageResize.
typedef NS_ENUM(NSInteger, XLSuperStarImageResize) {
    XLSuperStarImageResizeNone               = 0,
    XLSuperStarImageResizeExactWidth         = 1,
    XLSuperStarImageResizeExactHeight        = 2,
    XLSuperStarImageResizeExactWidthOrHeight = 3,
    XLSuperStarImageResizeAll                = 4,
};

// Layer-blend literals — one of these strings is what
// `T_CHOICE_LayerMethod` expects.
extern NSString* const XLSuperStarLayerBlendNormal;
extern NSString* const XLSuperStarLayerBlendAverage;
extern NSString* const XLSuperStarLayerBlend2Reveals1;

// Options bag for a SuperStar import. Defaults match the desktop dialog.
@interface XLSuperStarImportOptions : NSObject
@property (nonatomic, assign) int xSize;
@property (nonatomic, assign) int ySize;
@property (nonatomic, assign) int xOffset;
@property (nonatomic, assign) int yOffset;
@property (nonatomic, assign) XLSuperStarImageResize imageResize;
@property (nonatomic, copy)   NSString* layerBlend;
@property (nonatomic, assign) int timingOffsetMs;
// Optional override for the embedded-image group prefix. Empty → use the
// file basename. The desktop pops a text-entry dialog mid-parse; iPad
// has no equivalent so we resolve up front.
@property (nonatomic, copy)   NSString* imageGroupPrefix;
@end

// One-shot `.sup` import. SuperStar's surface is structurally nothing
// like `.xsq` (no source/dest tree, no channel mapping) — the user
// picks **one** target model and the parser fans every morph / image /
// flowy / scene / textAction / imageAction in the file out as effects
// on that model's layers. Hence: no session, no mapping UI — just a
// list-models call + an apply call.
@interface XLSuperStarImport : NSObject

// Full model + submodel/strand names (e.g. "Tree" or "Tree/Strand 1")
// for every Element in the active sequence the user can target. Mirrors
// the desktop dialog's dropdown population.
+ (NSArray<NSString*>*)availableTargetModelNamesForDocument:(XLSequenceDocument*)document
    NS_SWIFT_NAME(availableTargetModelNames(forDocument:));

// Apply a `.sup` import. Returns YES on success; populates `outError`
// on read / parse / target-resolution failure.
+ (BOOL)applyImportFromPath:(NSString*)path
            targetModelName:(NSString*)modelName
                    options:(XLSuperStarImportOptions*)options
                   document:(XLSequenceDocument*)document
                      error:(NSError* _Nullable * _Nullable)outError
    NS_SWIFT_NAME(applyImport(fromPath:targetModelName:options:document:));

@end

NS_ASSUME_NONNULL_END
