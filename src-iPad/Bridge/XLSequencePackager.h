//
//  XLSequencePackager.h
//  xLights-iPadLib
//
//  Mirrors desktop's Tools → Package Sequence (xLightsMain.cpp's
//  PackageSequence). Walks the in-memory sequence + supporting
//  managers and writes a self-contained `.xsqz` archive via the
//  shared core `SequencePackage::Pack`. The result lives in
//  NSTemporaryDirectory and is intended to be handed to the system
//  share sheet — callers are responsible for cleanup once the
//  share dismisses.
//

#pragma once

#import <Foundation/Foundation.h>

@class XLSequenceDocument;

NS_ASSUME_NONNULL_BEGIN

@interface XLSequencePackager : NSObject

// Pack the currently-loaded sequence on `document` into a `.xsqz`
// and return its file:// URL. `excludeAudio` / `excludeVideos`
// mirror the desktop `SequencePackOptions` toggles. `outWarnings`
// (optional) receives per-file failures that did not abort the
// pack (e.g. a referenced image that no longer exists on disk —
// the package is still written without it). Catastrophic failures
// (no sequence open, write failures, XML parse) return nil and
// populate `outError`.
//
// Synchronous; safe to call from a background queue.
+ (nullable NSURL*)packageSequenceForDocument:(XLSequenceDocument*)document
                                 excludeAudio:(BOOL)excludeAudio
                                excludeVideos:(BOOL)excludeVideos
                                     warnings:(NSArray<NSString*>* _Nullable * _Nullable)outWarnings
                                        error:(NSError* _Nullable * _Nullable)outError
    NS_SWIFT_NAME(packageSequence(for:excludeAudio:excludeVideos:warnings:));

@end

NS_ASSUME_NONNULL_END
