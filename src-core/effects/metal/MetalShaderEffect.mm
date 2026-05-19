/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"

#include "../../render/RenderBuffer.h"
#include "../EffectManager.h"
#include "../ShaderEffect.h"

#ifdef USE_GLES

#define GL_GLES_PROTOTYPES 1
#define GL_GLEXT_PROTOTYPES 1
#define EGL_EGL_PROTOTYPES 1
#define EGL_EGLEXT_PROTOTYPES 1
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_angle.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>

#import <Metal/Metal.h>

#include "../../graphics/GLContextManager.h"
#include "../OpenGLShaders.h"

#include <log.h>

// -----------------------------------------------------------------------
// SharedTexture — a texture that exists in both Metal and ANGLE GL,
// backed by the same GPU memory.
// -----------------------------------------------------------------------
struct SharedTexture {
    EGLImage eglImage = EGL_NO_IMAGE;
    id<MTLTexture> metalTexture = nil;
    id<MTLBuffer> metalBuffer = nil;   // non-nil when backed by a RenderBuffer's MTLBuffer (zero-copy)
    GLuint glTexture = 0;
    int width = 0;
    int height = 0;
};

// Get the MTLDevice that ANGLE is using
static id<MTLDevice> getANGLEMetalDevice(EGLDisplay display) {
    EGLAttrib deviceHandle = 0;
    if (!eglQueryDisplayAttribEXT(display, EGL_DEVICE_EXT, &deviceHandle) || !deviceHandle)
        return nil;
    EGLAttrib mtlDevicePtr = 0;
    if (!eglQueryDeviceAttribEXT((EGLDeviceEXT)deviceHandle, EGL_METAL_DEVICE_ANGLE, &mtlDevicePtr) || !mtlDevicePtr)
        return nil;
    return (__bridge id<MTLDevice>)(void*)mtlDevicePtr;
}

// Import an MTLTexture into ANGLE as a GL texture via EGLImage.
static bool importTextureToGL(EGLDisplay display, id<MTLTexture> metalTex, SharedTexture& out) {
    const EGLAttrib imageAttribs[] = { EGL_NONE };
    EGLImage image = eglCreateImage(display, EGL_NO_CONTEXT, EGL_METAL_TEXTURE_ANGLE,
                                    (EGLClientBuffer)(void*)metalTex, imageAttribs);
    if (image == EGL_NO_IMAGE) {
        spdlog::error("MetalShaderEffect: eglCreateImage failed: 0x{:X}", eglGetError());
        return false;
    }

    GLuint glTex = 0;
    glGenTextures(1, &glTex);
    glBindTexture(GL_TEXTURE_2D, glTex);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        spdlog::error("MetalShaderEffect: GL error after texture import: 0x{:X}", err);
        eglDestroyImage(display, image);
        glDeleteTextures(1, &glTex);
        return false;
    }

    out.eglImage = image;
    out.glTexture = glTex;
    return true;
}

static void destroySharedTexture(EGLDisplay display, SharedTexture& tex) {
    if (tex.glTexture) { glDeleteTextures(1, &tex.glTexture); tex.glTexture = 0; }
    if (tex.eglImage != EGL_NO_IMAGE && display) { eglDestroyImage(display, tex.eglImage); tex.eglImage = EGL_NO_IMAGE; }
    if (tex.metalTexture) { [tex.metalTexture release]; tex.metalTexture = nil; }
    if (tex.metalBuffer) { [tex.metalBuffer release]; tex.metalBuffer = nil; }
    tex.width = 0;
    tex.height = 0;
}

// Create a standalone shared texture (ANGLE allocates the storage).
static bool createSharedTexture(EGLDisplay display, int w, int h, SharedTexture& out) {
    id<MTLDevice> device = getANGLEMetalDevice(display);
    if (!device) return false;

    MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                   width:w height:h mipmapped:NO];
    desc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite | MTLTextureUsageRenderTarget;
    desc.storageMode = MTLStorageModeShared;

    id<MTLTexture> metalTex = [device newTextureWithDescriptor:desc];
    if (!metalTex) return false;

    if (!importTextureToGL(display, metalTex, out)) {
        [metalTex release];
        return false;
    }

    out.metalTexture = metalTex;
    out.metalBuffer = nil;
    out.width = w;
    out.height = h;
    return true;
}

// Create a shared texture backed by a RenderBuffer's existing Metal pixel buffer (zero-copy).
static bool createSharedTextureFromRenderBuffer(EGLDisplay display, void* gpuRenderData,
                                                 int w, int h, SharedTexture& out) {
    if (!gpuRenderData) return false;

    id<MTLDevice> device = getANGLEMetalDevice(display);
    if (!device) return false;

    auto* rbcd = static_cast<MetalRenderBufferComputeData*>(gpuRenderData);
    id<MTLBuffer> pixelBuffer = rbcd->getPixelBuffer(false);
    if (!pixelBuffer) return false;

    if (pixelBuffer.device.registryID != device.registryID) return false;

    NSUInteger bytesPerRow = w * 4;
    NSUInteger minAlignment = [device minimumLinearTextureAlignmentForPixelFormat:MTLPixelFormatRGBA8Unorm];
    if (bytesPerRow % minAlignment != 0) return false;
    if (pixelBuffer.length < bytesPerRow * h) return false;

    MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                   width:w height:h mipmapped:NO];
    desc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite | MTLTextureUsageRenderTarget;
    desc.storageMode = MTLStorageModeShared;

    id<MTLTexture> metalTex = [pixelBuffer newTextureWithDescriptor:desc offset:0 bytesPerRow:bytesPerRow];
    if (!metalTex) return false;

    if (!importTextureToGL(display, metalTex, out)) {
        [metalTex release];
        return false;
    }

    out.metalTexture = metalTex;
    out.metalBuffer = [pixelBuffer retain];
    out.width = w;
    out.height = h;

    spdlog::info("MetalShaderEffect: ZERO-COPY shared texture {}x{} from RenderBuffer (GL={})", w, h, out.glTexture);
    return true;
}

// -----------------------------------------------------------------------
// MetalShaderEffectCache — per-RenderBuffer cache for the Metal interop state
// -----------------------------------------------------------------------
class MetalShaderEffectCache : public EffectRenderCache {
public:
    MetalShaderEffectCache() {}
    virtual ~MetalShaderEffectCache() {
        EGLDisplay display = (EGLDisplay)GLContextManager::Instance().GetNativeDisplay();
        destroySharedTexture(display, sharedInputTex);
        destroySharedTexture(display, sharedOutputTex);
    }

    SharedTexture sharedInputTex;
    SharedTexture sharedOutputTex;
    bool useMetalInterop = false;
};

static MetalShaderEffectCache* getMetalCache(int effectId, RenderBuffer& buffer) {
    int slot = effectId + EffectManager::eff_LASTEFFECT;
    auto* cache = static_cast<MetalShaderEffectCache*>(buffer.infoCache[slot]);
    if (!cache) {
        cache = new MetalShaderEffectCache();
        buffer.infoCache[slot] = cache;
    }
    return cache;
}

#endif // USE_GLES

// -----------------------------------------------------------------------
// MetalShaderEffect implementation
// -----------------------------------------------------------------------

MetalShaderEffect::MetalShaderEffect(int i) : ShaderEffect(i) {
}

MetalShaderEffect::~MetalShaderEffect() {
}

void MetalShaderEffect::preparePixelTextures(RenderBuffer& buffer, bool shadersInit, unsigned fbId) {
#ifdef USE_GLES
    auto* cache = getMetalCache(id, buffer);

    // Recreate shared textures if the buffer was resized
    if (cache->useMetalInterop &&
        (cache->sharedInputTex.width != buffer.BufferWi || cache->sharedInputTex.height != buffer.BufferHt)) {
        EGLDisplay display = (EGLDisplay)GLContextManager::Instance().GetNativeDisplay();
        destroySharedTexture(display, cache->sharedInputTex);
        destroySharedTexture(display, cache->sharedOutputTex);
        cache->useMetalInterop = false;
    }

    if (!cache->useMetalInterop && shadersInit) {
        EGLDisplay display = (EGLDisplay)GLContextManager::Instance().GetNativeDisplay();
        if (display) {
            bool inputOk = createSharedTextureFromRenderBuffer(display, buffer.gpuRenderData,
                                                                buffer.BufferWi, buffer.BufferHt, cache->sharedInputTex);
            bool outputOk = createSharedTextureFromRenderBuffer(display, buffer.gpuRenderData,
                                                                 buffer.BufferWi, buffer.BufferHt, cache->sharedOutputTex);
            if (!inputOk)
                inputOk = createSharedTexture(display, buffer.BufferWi, buffer.BufferHt, cache->sharedInputTex);
            if (!outputOk)
                outputOk = createSharedTexture(display, buffer.BufferWi, buffer.BufferHt, cache->sharedOutputTex);

            if (inputOk && outputOk) {
                LOG_GL_ERRORV(glBindFramebuffer(GL_FRAMEBUFFER, fbId));
                LOG_GL_ERRORV(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                      GL_TEXTURE_2D, cache->sharedOutputTex.glTexture, 0));
                GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                if (status == GL_FRAMEBUFFER_COMPLETE) {
                    cache->useMetalInterop = true;
                    spdlog::info("MetalShaderEffect: interop enabled ({}x{}, input:{}, output:{})",
                                 buffer.BufferWi, buffer.BufferHt,
                                 cache->sharedInputTex.metalBuffer ? "zero-copy" : "Metal-copy",
                                 cache->sharedOutputTex.metalBuffer ? "zero-copy" : "Metal-copy");
                    // Attach already done above; skip the per-frame re-attach below.
                    return;
                } else {
                    spdlog::warn("MetalShaderEffect: FBO incomplete (0x{:X}), falling back", status);
                    destroySharedTexture(display, cache->sharedInputTex);
                    destroySharedTexture(display, cache->sharedOutputTex);
                }
                LOG_GL_ERRORV(glBindFramebuffer(GL_FRAMEBUFFER, 0));
            } else {
                if (inputOk) destroySharedTexture(display, cache->sharedInputTex);
                if (outputOk) destroySharedTexture(display, cache->sharedOutputTex);
            }
        }
    }

    // FBO is recreated per-frame in Render(), so re-attach the shared
    // output texture each frame the interop path is active.
    if (cache->useMetalInterop) {
        LOG_GL_ERRORV(glBindFramebuffer(GL_FRAMEBUFFER, fbId));
        LOG_GL_ERRORV(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              GL_TEXTURE_2D, cache->sharedOutputTex.glTexture, 0));
    }
#endif
}

void MetalShaderEffect::copyPixelDataToTexture(RenderBuffer& buffer, unsigned rbTex) {
#ifdef USE_GLES
    auto* cache = getMetalCache(id, buffer);
    if (cache->useMetalInterop) {
        // If buffer-backed (zero-copy), pixels are already in the texture — just bind it.
        // Otherwise, upload via Metal's replaceRegion.
        if (!cache->sharedInputTex.metalBuffer) {
            MTLRegion region = MTLRegionMake2D(0, 0, buffer.BufferWi, buffer.BufferHt);
            [cache->sharedInputTex.metalTexture replaceRegion:region mipmapLevel:0
                                                    withBytes:buffer.GetPixels()
                                                  bytesPerRow:buffer.BufferWi * 4];
        }
        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, cache->sharedInputTex.glTexture));
        return;
    }
#endif
    ShaderEffect::copyPixelDataToTexture(buffer, rbTex);
}

void MetalShaderEffect::copyPixelDataFromTexture(RenderBuffer& buffer) {
#ifdef USE_GLES
    auto* cache = getMetalCache(id, buffer);
    if (cache->useMetalInterop) {
        // If buffer-backed (zero-copy), result is already in pixels — just flush GL.
        // Otherwise, download via Metal's getBytes.
        glFinish();
        if (!cache->sharedOutputTex.metalBuffer) {
            MTLRegion region = MTLRegionMake2D(0, 0, buffer.BufferWi, buffer.BufferHt);
            [cache->sharedOutputTex.metalTexture getBytes:buffer.GetPixels()
                                              bytesPerRow:buffer.BufferWi * 4
                                               fromRegion:region mipmapLevel:0];
        }
        return;
    }
#endif
    ShaderEffect::copyPixelDataFromTexture(buffer);
}
