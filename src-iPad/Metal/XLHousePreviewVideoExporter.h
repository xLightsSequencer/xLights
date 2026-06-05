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

// Offscreen house-preview → video exporter for iPad. Renders every model of
// the active layout group at an arbitrary target size (e.g. 1920×1080) into a
// Metal texture per frame — independent of the on-screen preview's size —
// reads each frame back to RGB24, and encodes via the core VideoWriter
// (AVFoundation H.264 / HEVC). Mirrors the desktop ExportVideoPreview flow but
// drives the projection at the requested resolution rather than the live pane.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface XLHousePreviewVideoExporter : NSObject

// Render frames [startFrame, endFrame) of the house preview and encode to
// `path` (`.mp4`). `renderContext` is an `iPadRenderContext*` (as void*).
// `is3d` picks the 2D-ortho vs 3D-perspective projection (use the show's saved
// layout mode). `highQuality` selects HEVC constant-quality over H.264.
// `progress` is invoked with a 0..1 fraction during the encode, on the calling
// thread. Runs synchronously and is slow (full-show length × full resolution)
// — invoke from a background queue. Returns YES on success.
+ (BOOL)exportToPath:(NSString*)path
       renderContext:(void*)renderContext
               width:(int)width
              height:(int)height
                is3d:(BOOL)is3d
         highQuality:(BOOL)highQuality
          startFrame:(int)startFrame
            endFrame:(int)endFrame
            progress:(nullable void (^)(double fraction))progress;

@end

NS_ASSUME_NONNULL_END
