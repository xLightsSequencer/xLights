//
//  XLLogPackager.h
//  xLights-iPadLib
//
//  Builds a "Package Logs" zip suitable for sharing via
//  UIActivityViewController. Bundles xLights.log + rotated siblings,
//  MetricKit diagnostics JSON, the active show-folder XML, the
//  currently-loaded sequence, a threads dump, and a device-info
//  sidecar.
//

#pragma once

#import <Foundation/Foundation.h>

@class XLSequenceDocument;

NS_ASSUME_NONNULL_BEGIN

@interface XLLogPackager : NSObject

// Build the zip and return its file:// URL. `document` may be nil
// (no show folder loaded). The returned zip lives in NSTemporaryDirectory
// — the caller should delete it once the share sheet dismisses.
//
// Synchronous; safe to call from a background queue. Throws on
// unrecoverable failure (e.g. NSTemporaryDirectory write error).
+ (nullable NSURL*)packageLogsForDocument:(nullable XLSequenceDocument*)document
                                    error:(NSError* _Nullable* _Nullable)outError
    NS_SWIFT_NAME(packageLogs(for:));

// Smaller-payload variant for the auto-upload pipeline. Excludes the
// show folder XML and the open sequence — only logs, MetricKit JSON,
// threads.txt, and device-info.txt. Writes directly into
// Library/Logs/PendingUpload/ so the uploader can sweep it on next
// activation. Returns the file URL of the staged zip, or nil on
// failure.
+ (nullable NSURL*)stagePendingUploadWithError:(NSError* _Nullable* _Nullable)outError
    NS_SWIFT_NAME(stagePendingUpload());

@end

NS_ASSUME_NONNULL_END
