#pragma once

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

@class XLSequenceDocument;

// ObjC bridge for xlStandaloneMetalCanvas + iPadModelPreview.
// Owns the C++ canvas and preview, drives model rendering.

@interface XLMetalBridge : NSObject

- (instancetype)initWithName:(NSString*)name;
- (void)attachLayer:(CAMetalLayer*)layer;
- (void)setDrawableSize:(CGSize)size scale:(CGFloat)scale;

// Draw all models at the given frame time using DisplayEffectOnWindow.
// When setPreviewModel: has set a non-empty name, only that model (plus view
// objects) is drawn — everything else is skipped. Empty/nil preview model =
// full house preview behavior.
- (void)drawModelsForDocument:(XLSequenceDocument*)doc atMS:(int)frameMS pointSize:(float)pointSize;

// Preview-mode model filter. Empty/nil string clears the filter.
- (void)setPreviewModel:(NSString*)modelName;

// Camera controls — route through the active PreviewCamera on iPadModelPreview.
// Zoom: 1.0 default, >1 zooms in. Pan is in world units. Rotate is in degrees.
- (void)setCameraZoom:(float)zoom;
- (float)cameraZoom;
- (void)setCameraPanX:(float)x panY:(float)y;
- (void)offsetCameraPanX:(float)dx panY:(float)dy;
- (float)cameraPanX;
- (float)cameraPanY;
- (void)setCameraAngleX:(float)ax angleY:(float)ay;
- (void)offsetCameraAngleX:(float)dx angleY:(float)dy;
- (float)cameraAngleX;
- (float)cameraAngleY;
- (void)resetCamera;

// 2D/3D mode. True = perspective (3D); false = orthographic (2D).
- (void)setIs3D:(BOOL)is3d;
- (BOOL)is3D;

// Coarse view-object (house mesh / terrain / gridlines / ground images)
// visibility toggle for the House Preview. No effect on Model Preview —
// that pane never draws view objects. Default: YES.
- (void)setShowViewObjects:(BOOL)show;
- (BOOL)showViewObjects;

// Drop the cached background texture. Called when the active layout
// group changes (possibly new background image path). Next draw
// re-loads whatever the render context's active-group path is.
- (void)invalidateBackgroundCache;

// Viewpoints (saved camera positions). List is filtered to the pane's
// current 2D/3D mode. Apply copies a saved PreviewCamera into the
// active camera; saveAs captures the active camera under a new name.
// Persistence for the add/delete mutations requires the document — the
// document triggers the rgbeffects.xml rewrite.
- (NSArray<NSString*>*)viewpointNamesForDocument:(XLSequenceDocument*)doc;
- (BOOL)applyViewpointNamed:(NSString*)name
                forDocument:(XLSequenceDocument*)doc;
- (BOOL)saveCurrentViewAs:(NSString*)name
              forDocument:(XLSequenceDocument*)doc;
- (BOOL)deleteViewpointNamed:(NSString*)name
                 forDocument:(XLSequenceDocument*)doc;
- (void)restoreDefaultViewpointForDocument:(XLSequenceDocument*)doc;

// Fit-to-window camera shortcuts. Fit All frames every model in the
// active layout group (matching the House Preview filter); Fit Model
// frames the single named model (or returns NO if it's missing or
// hidden by the current layout group). Adjusts zoom + pan (2D) or
// distance + pan (3D) without changing rotation angles.
- (BOOL)fitAllModelsForDocument:(XLSequenceDocument*)doc;
- (BOOL)fitModelNamed:(NSString*)name
          forDocument:(XLSequenceDocument*)doc;

@end
