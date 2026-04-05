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

// IMetalCanvas — wx-free abstract interface for a Metal rendering canvas.
// Implemented by xlMetalCanvas (desktop wx) and future iPad canvas.
// Used by xlMetalGraphicsContext so it has no dependency on wx types.

#include <string>

#include "../../utils/Color.h"

#ifdef __OBJC__
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#endif

class IMetalCanvas {
public:
    virtual ~IMetalCanvas() = default;

    // Identity / naming
    virtual std::string getName() const = 0;

    // Rendering properties
    virtual xlColor ClearBackgroundColor() const = 0;
    virtual bool usesMSAA() = 0;
    virtual bool RequiresDepthBuffer() const = 0;
    virtual bool drawingUsingLogicalSize() const = 0;
    virtual double translateToBacking(double x) const = 0;

#ifdef __OBJC__
    // Shared Metal resources (delegates to MetalDeviceManager)
    virtual id<MTLDevice>            getMTLDevice() = 0;
    virtual id<MTLCommandQueue>      getMTLCommandQueue() = 0;
    virtual id<MTLCommandQueue>      getBltCommandQueue() = 0;
    virtual id<MTLLibrary>           getMTLLibrary() = 0;
    virtual int                      getMSAASampleCount() = 0;

    // Per-canvas textures
    virtual id<MTLTexture> getMSAATexture(int w, int h) = 0;
    virtual id<MTLTexture> getDepthTexture(int w, int h) = 0;

    // Depth stencil states (shared, but accessed through canvas)
    virtual id<MTLDepthStencilState> getDepthStencilStateLE() = 0;
    virtual id<MTLDepthStencilState> getDepthStencilStateL() = 0;

    // Pipeline state management
    virtual id<MTLRenderPipelineState> getPipelineState(const std::string& name,
                                                        const char* vShader,
                                                        const char* fShader,
                                                        bool blending) = 0;

    // Frame presentation
    virtual void addToSyncPoint(id<MTLCommandBuffer>& buffer,
                                id<CAMetalDrawable>& drawable) = 0;

    // Drawable access — desktop gets this from MTKView, iPad from CAMetalLayer
    virtual id<CAMetalDrawable> getNextDrawable() = 0;
#endif
};
