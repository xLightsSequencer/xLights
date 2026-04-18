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

@end
