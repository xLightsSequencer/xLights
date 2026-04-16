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

// Draw all models at the given frame time using DisplayEffectOnWindow
- (void)drawModelsForDocument:(XLSequenceDocument*)doc atMS:(int)frameMS pointSize:(float)pointSize;

@end
