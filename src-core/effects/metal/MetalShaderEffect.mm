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
#include "MetalShaderTranslator.h"

#include "../../render/RenderBuffer.h"
#include "../EffectManager.h"
#include "../ShaderEffect.h"
#include "UtilClasses.h"       // full SettingsMap definition (for .Get/.GetInt/.GetBool)
#include "../../render/SequenceElements.h" // EffectLayer/Effect for EVENT parms (as the base does)
#include "../../media/AudioManager.h"      // FFT frame data for audio shaders

#import <Metal/Metal.h>
#include <TargetConditionals.h>
#include <array>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <mutex>
#include <unordered_map>

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
#include <mutex>

// ANGLE's Metal backend and the shared EGLImage/Metal device it imports
// into are not thread-safe, and the size-1 GL context pool does not
// actually serialize the Metal-side calls here (getBytes/replaceRegion
// and texture creation don't require the GL context to be current). The
// render engine runs ShaderEffect::Render on many worker threads at once
// (per-model jobs plus per-sub-buffer parallel_for), so without this
// lock one thread can create/replace a shared texture while others read
// it via getBytes — a data race that crashes in createSharedTexture
// (sig 5d9f29a77c). Serialize every interop touch through one mutex.
static std::mutex sMetalInteropMutex;

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
                                    (EGLClientBuffer)(__bridge void*)metalTex, imageAttribs);
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
    tex.metalTexture = nil;
    tex.metalBuffer = nil;
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
        return false;
    }

    out.metalTexture = metalTex;
    out.metalBuffer = pixelBuffer;
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
        // Freeing the shared textures hits ANGLE/EGL + Metal, which can
        // race a concurrent createSharedTexture on another render thread
        // (sig 5d9f29a77c) — serialize through the same interop mutex.
        std::lock_guard<std::mutex> interopLock(sMetalInteropMutex);
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
    std::lock_guard<std::mutex> interopLock(sMetalInteropMutex);
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
    std::lock_guard<std::mutex> interopLock(sMetalInteropMutex);
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
    std::lock_guard<std::mutex> interopLock(sMetalInteropMutex);
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

// -----------------------------------------------------------------------
// Native Metal render path (opt-in via XL_NATIVE_SHADER)
// -----------------------------------------------------------------------

namespace {

// Per-RenderBuffer cache for the native pipeline + config. Distinct slot from
// the base ShaderRenderCache (infoCache[id]) and the ANGLE interop cache.
using UniformValues = std::unordered_map<std::string, std::array<float, 4>>;

// Process-wide program cache keyed by the assembled fragment source (the vertex
// shader is fixed). Translation + MSL compile + pipeline build cost ~40ms per
// shader; without this every (effect x buffer) pays it. Mirrors the GL path's
// source-keyed shaderMap. Failures are cached too so a bad shader doesn't
// re-translate per buffer.
struct CachedShaderProgram {
    id<MTLRenderPipelineState> pso = nil;
    ShaderTranslate::ShaderStageInfo vsInfo;
    ShaderTranslate::ShaderStageInfo fsInfo;
    bool ok = false;
};
static std::mutex sProgramCacheMutex;
static std::unordered_map<std::string, CachedShaderProgram>& programCache() {
    static std::unordered_map<std::string, CachedShaderProgram> cache;
    return cache;
}
static constexpr NSUInteger kVertexSlot = 30; // vertex data slot, above the uniform slots

static CachedShaderProgram buildShaderProgram(const std::string& fragmentSource, const std::string& label) {
    static const bool dbg = getenv("XL_NATIVE_SHADER_DEBUG") != nullptr;
    CachedShaderProgram out;
    id<MTLDevice> device = MetalComputeUtilities::INSTANCE.device;
#if TARGET_OS_IPHONE
    const bool forIOS = true;
#else
    const bool forIOS = false;
#endif
    auto prog = ShaderTranslate::TranslateProgram(ShaderEffect::GetNativeVertexShaderSource(), fragmentSource, forIOS);
    if (!prog.ok) {
        if (dbg) fprintf(stderr, "NATIVE xlatefail %s: %s\n", label.c_str(), prog.error.substr(0, 300).c_str());
        return out;
    }
    if (dbg) {
        fprintf(stderr, "NATIVE build %s: attrVpos=%d attrTpos=%d fsSampler=%d vsU=%zu fsU=%zu\n",
                label.c_str(), prog.vertex.attrVpos, prog.vertex.attrTpos,
                prog.fragment.samplerTexture, prog.vertex.uniforms.size(), prog.fragment.uniforms.size());
    }
    NSError* e = nil;
    id<MTLLibrary> vlib = [device newLibraryWithSource:[NSString stringWithUTF8String:prog.vertexMSL.c_str()] options:nil error:&e];
    id<MTLLibrary> flib = [device newLibraryWithSource:[NSString stringWithUTF8String:prog.fragmentMSL.c_str()] options:nil error:&e];
    id<MTLFunction> vfn = [vlib newFunctionWithName:@"main0"];
    id<MTLFunction> ffn = [flib newFunctionWithName:@"main0"];
    if (vfn == nil || ffn == nil) {
        if (dbg) fprintf(stderr, "NATIVE msllibfail %s: %s\n", label.c_str(), e ? e.localizedDescription.UTF8String : "?");
        return out;
    }
    MTLRenderPipelineDescriptor* rpd = [[MTLRenderPipelineDescriptor alloc] init];
    rpd.vertexFunction = vfn;
    rpd.fragmentFunction = ffn;
    rpd.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA8Unorm;
    MTLVertexDescriptor* vd = [MTLVertexDescriptor vertexDescriptor];
    if (prog.vertex.attrVpos >= 0) {
        vd.attributes[prog.vertex.attrVpos].format = MTLVertexFormatFloat2;
        vd.attributes[prog.vertex.attrVpos].offset = 0;
        vd.attributes[prog.vertex.attrVpos].bufferIndex = kVertexSlot;
    }
    if (prog.vertex.attrTpos >= 0) {
        vd.attributes[prog.vertex.attrTpos].format = MTLVertexFormatFloat2;
        vd.attributes[prog.vertex.attrTpos].offset = 8;
        vd.attributes[prog.vertex.attrTpos].bufferIndex = kVertexSlot;
    }
    vd.layouts[kVertexSlot].stride = 16;
    vd.layouts[kVertexSlot].stepFunction = MTLVertexStepFunctionPerVertex;
    rpd.vertexDescriptor = vd;
    out.pso = [device newRenderPipelineStateWithDescriptor:rpd error:&e];
    if (out.pso == nil) return out;
    out.vsInfo = prog.vertex;
    out.fsInfo = prog.fragment;
    out.ok = true;
    return out;
}

// Holds the native pipeline + config and does the Metal work. Kept off
// MetalShaderEffect because that class's inherited `int id` data member shadows
// the ObjC `id` type inside its methods (as it would here otherwise).
class MetalShaderNativeCache : public EffectRenderCache {
public:
    MetalShaderNativeCache() {}
    virtual ~MetalShaderNativeCache() { delete config; } // ARC releases the ObjC members

    ShaderConfig* config = nullptr;
    std::string shaderFile; // cache key: rebuild when the effect's .fs changes
    long timeMS = 0;
    int width = 0;
    int height = 0;
    bool built = false;
    bool failed = false; // translation/pipeline unsupported -> GL fallback

    void reset() {
        delete config;
        config = nullptr;
        built = false;
        failed = false;
        pso = nil;
        outputTex = nil;
        inputTex = nil;
        audioTex = nil;
    }

    // Fetch (or build once, process-wide) the translated program, then create
    // the per-buffer-size resources.
    bool build(RenderBuffer& buffer) {
        id<MTLDevice> device = MetalComputeUtilities::INSTANCE.device;
        CachedShaderProgram prog;
        {
            std::lock_guard<std::mutex> lock(sProgramCacheMutex);
            auto& cache = programCache();
            auto it = cache.find(config->GetCode());
            if (it == cache.end()) {
                it = cache.emplace(config->GetCode(), buildShaderProgram(config->GetCode(), config->GetFilename())).first;
            }
            prog = it->second;
        }
        if (!prog.ok) return false;
        pso = prog.pso;
        vsInfo = prog.vsInfo;
        fsInfo = prog.fsInfo;
        // tpos.y is inverted relative to the GL quad: Metal render targets are
        // top-row-first while the RenderBuffer (GL convention) is bottom-row-
        // first, and the blit copies texture row N to buffer row N. Flipping the
        // texture coordinate here keeps shader-space y consistent with GL.
        static const float quad[] = { 1.f, -1.f, 1.f, 1.f,  -1.f, -1.f, 0.f, 1.f,
                                       1.f,  1.f, 1.f, 0.f,  -1.f,  1.f, 0.f, 0.f };
        quadBuffer = [device newBufferWithBytes:quad length:sizeof(quad) options:MTLResourceStorageModeShared];
        MTLSamplerDescriptor* sd = [[MTLSamplerDescriptor alloc] init];
        sd.minFilter = MTLSamplerMinMagFilterLinear;
        sd.magFilter = MTLSamplerMinMagFilterLinear;
        sd.sAddressMode = MTLSamplerAddressModeClampToEdge;
        sd.tAddressMode = MTLSamplerAddressModeClampToEdge;
        sampler = [device newSamplerStateWithDescriptor:sd];
        MTLTextureDescriptor* itd = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                      width:buffer.BufferWi height:buffer.BufferHt mipmapped:NO];
        itd.usage = MTLTextureUsageShaderRead;
        itd.storageMode = MTLStorageModeShared;
        inputTex = [device newTextureWithDescriptor:itd];

        if (config->IsAudioFFTShader() || config->IsAudioIntensityShader()) {
            // Matches the GL FFTAudioTexture: 128x1 single-channel float,
            // bound in place of the pixel input texture.
            MTLTextureDescriptor* atd = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR32Float
                                                                                          width:128 height:1 mipmapped:NO];
            atd.usage = MTLTextureUsageShaderRead;
            atd.storageMode = MTLStorageModeShared;
            audioTex = [device newTextureWithDescriptor:atd];
        }

        // Render target: a normal tiled texture (buffer-backed linear textures
        // can't be render targets on many GPUs). Blitted into the pixel buffer
        // after each frame. Kept across frames so overlay (loadAction Load) reads
        // the prior frame.
        MTLTextureDescriptor* otd = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                      width:buffer.BufferWi height:buffer.BufferHt mipmapped:NO];
        otd.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
        otd.storageMode = MTLStorageModePrivate;
        outputTex = [device newTextureWithDescriptor:otd];

        width = buffer.BufferWi;
        height = buffer.BufferHt;
        built = true;
        return true;
    }

    // Encode one frame into the RenderBuffer's own pixel MTLBuffer. Returns false
    // (aborting any started command buffer) if it can't run this frame so the
    // caller falls back to the GL path.
    bool encode(RenderBuffer& buffer, const UniformValues& vals, bool overlay) {
        static const bool dbg = getenv("XL_NATIVE_SHADER_DEBUG") != nullptr;
        MetalRenderBufferComputeData* rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
        if (rbcd == nullptr) {
            if (dbg && (buffer.curPeriod % 100) == 0) {
                fprintf(stderr, "NATIVE encodefail rb=%p model=%s f=%d %s: no rbcd\n",
                        (void*)&buffer, buffer.GetModelName().c_str(),
                        buffer.curPeriod, shaderFile.c_str());
            }
            return false;
        }
        id<MTLCommandBuffer> commandBuffer = rbcd->getCommandBuffer();
        if (commandBuffer == nil) {
            if (dbg) fprintf(stderr, "NATIVE encodefail f=%d %s: no command buffer\n", buffer.curPeriod, shaderFile.c_str());
            return false;
        }
        id<MTLBuffer> pixelBuffer = rbcd->getPixelBuffer();
        const NSUInteger bpr = buffer.BufferWi * 4;
        if (pixelBuffer == nil || pixelBuffer.length < bpr * buffer.BufferHt) {
            if (dbg) fprintf(stderr, "NATIVE encodefail f=%d %s: pixbuf len=%lu need=%lu\n", buffer.curPeriod, shaderFile.c_str(),
                             (unsigned long)(pixelBuffer ? pixelBuffer.length : 0), (unsigned long)(bpr * buffer.BufferHt));
            rbcd->abortCommandBuffer();
            return false;
        }
        const bool usesTexture = fsInfo.samplerTexture >= 0;
        const bool audioMode = audioTex != nil;
        if (usesTexture && audioMode) {
            // Audio shaders sample the FFT/intensity texture instead of the
            // pixel input — same 128-float layout the GL path uploads.
            float fft[128] = { 0 };
            AudioManager* audioManager = buffer.GetMedia();
            if (audioManager != nullptr) {
                auto fftData = audioManager->GetFrameData(buffer.curPeriod, "");
                if (fftData) {
                    if (config->IsAudioFFTShader()) {
                        const size_t n = std::min<size_t>(fftData->vu.size(), 128);
                        for (size_t i = 0; i < n; i++) fft[i] = fftData->vu[i];
                    } else {
                        for (int i = 0; i < 127; i++) fft[i] = fftData->max;
                    }
                }
            }
            [audioTex replaceRegion:MTLRegionMake2D(0, 0, 128, 1)
                        mipmapLevel:0 withBytes:fft bytesPerRow:128 * 4];
        } else if (usesTexture) {
            // Feedback: upload a stable snapshot of the prior contents (the pixel
            // buffer, from last frame's blit) into the input texture.
            [inputTex replaceRegion:MTLRegionMake2D(0, 0, buffer.BufferWi, buffer.BufferHt)
                        mipmapLevel:0 withBytes:buffer.GetPixels() bytesPerRow:bpr];
        }
        MTLRenderPassDescriptor* rp = [MTLRenderPassDescriptor renderPassDescriptor];
        rp.colorAttachments[0].texture = outputTex;
        rp.colorAttachments[0].loadAction = overlay ? MTLLoadActionLoad : MTLLoadActionClear;
        rp.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0);
        rp.colorAttachments[0].storeAction = MTLStoreActionStore;
        if (dbg && (buffer.curPeriod % 100) == 0) {
            for (const auto& u : fsInfo.uniforms) {
                auto vit = vals.find(u.name);
                fprintf(stderr, "NATIVE bind f=%d %s: %s@%u = %s%.4f\n", buffer.curPeriod, shaderFile.c_str(),
                        u.name.c_str(), u.index, vit == vals.end() ? "MISSING " : "",
                        vit == vals.end() ? 0.0 : vit->second[0]);
            }
        }
        id<MTLRenderCommandEncoder> enc = [commandBuffer renderCommandEncoderWithDescriptor:rp];
        [enc setLabel:@"NativeShaderEffect"];
        [enc setRenderPipelineState:pso];
        [enc setVertexBuffer:quadBuffer offset:0 atIndex:kVertexSlot];
        bindStage(enc, vsInfo, vals, true);
        bindStage(enc, fsInfo, vals, false);
        if (usesTexture) {
            [enc setFragmentTexture:(audioMode ? audioTex : inputTex) atIndex:fsInfo.samplerTexture];
            [enc setFragmentSamplerState:sampler atIndex:fsInfo.samplerTexture];
        }
        [enc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
        [enc endEncoding];

        // Copy the rendered tiled texture into the RenderBuffer's pixel buffer
        // (= buffer.pixels). The framework commits + waits on commandBuffer.
        id<MTLBlitCommandEncoder> blit = [commandBuffer blitCommandEncoder];
        [blit copyFromTexture:outputTex sourceSlice:0 sourceLevel:0
                 sourceOrigin:MTLOriginMake(0, 0, 0)
                   sourceSize:MTLSizeMake(buffer.BufferWi, buffer.BufferHt, 1)
                     toBuffer:pixelBuffer destinationOffset:0
        destinationBytesPerRow:bpr destinationBytesPerImage:bpr * buffer.BufferHt];
        [blit endEncoding];
        return true;
    }

private:
    id<MTLRenderPipelineState> pso = nil;
    id<MTLSamplerState> sampler = nil;
    id<MTLBuffer> quadBuffer = nil;
    id<MTLTexture> inputTex = nil;
    id<MTLTexture> outputTex = nil;
    id<MTLTexture> audioTex = nil; // 128x1 R32F FFT/intensity, replaces inputTex for audio shaders
    ShaderTranslate::ShaderStageInfo vsInfo;
    ShaderTranslate::ShaderStageInfo fsInfo;

    static void bindStage(id<MTLRenderCommandEncoder> enc, const ShaderTranslate::ShaderStageInfo& info,
                          const UniformValues& vals, bool vertex) {
        for (const auto& u : info.uniforms) {
            auto vit = vals.find(u.name);
            if (vit == vals.end()) continue;
            uint32_t tmp[4] = { 0, 0, 0, 0 };
            for (int i = 0; i < u.vecSize && i < 4; i++) {
                if (u.isFloat) {
                    float f = vit->second[i];
                    memcpy(&tmp[i], &f, 4);
                } else {
                    int iv = (int)std::lround(vit->second[i]);
                    memcpy(&tmp[i], &iv, 4);
                }
            }
            NSUInteger len = (NSUInteger)u.vecSize * 4;
            if (vertex) {
                [enc setVertexBytes:tmp length:len atIndex:u.index];
            } else {
                [enc setFragmentBytes:tmp length:len atIndex:u.index];
            }
        }
    }
};

static MetalShaderNativeCache* getNativeCache(int effectId, RenderBuffer& buffer) {
    int slot = effectId + 2 * EffectManager::eff_LASTEFFECT;
    auto* c = static_cast<MetalShaderNativeCache*>(buffer.infoCache[slot]);
    if (!c) {
        c = new MetalShaderNativeCache();
        buffer.infoCache[slot] = c;
    }
    return c;
}

} // namespace

void MetalShaderEffect::Render(Effect* eff, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    // Native Metal is the default shader path on macOS and iPad;
    // XL_NO_NATIVE_SHADER=1 falls back to the OpenGL/ANGLE path
    // (diagnostics / comparison).
    static const bool nativeEnabled = getenv("XL_NO_NATIVE_SHADER") == nullptr;
    if (!nativeEnabled) {
        ShaderEffect::Render(eff, SettingsMap, buffer);
        return;
    }
    auto* cache = getNativeCache(id, buffer);
    const std::string shaderFile = SettingsMap.Get("0FILEPICKERCTRL_IFS", "");
    if (shaderFile.empty()) {
        ShaderEffect::Render(eff, SettingsMap, buffer);
        return;
    }
    // A new effect (or a different shader file) on the same buffer must not
    // reuse the previous effect's compiled pipeline/config — or its failed
    // flag. The infoCache slot is per (buffer, effect TYPE), not per instance,
    // so this reset must run BEFORE the failed check.
    if (cache->shaderFile != shaderFile) {
        cache->reset();
        cache->shaderFile = shaderFile;
    }
    if (cache->failed) {
        ShaderEffect::Render(eff, SettingsMap, buffer);
        return;
    }

    @autoreleasepool {
        const bool effectStart = buffer.needToInit;

        // ---- build config + pipeline on first frame / file change / resize ----
        if (!cache->built || cache->width != buffer.BufferWi || cache->height != buffer.BufferHt) {
            if (!cache->config) {
                cache->config = ShaderEffect::ParseShader(shaderFile, GetSequenceElements(buffer));
            }
            if (!cache->config || cache->config->GetCode().empty()) {
                cache->failed = true;
                ShaderEffect::Render(eff, SettingsMap, buffer);
                return;
            }
            cache->built = false;
            if (!cache->build(buffer)) {
                cache->failed = true;
                ShaderEffect::Render(eff, SettingsMap, buffer);
                return;
            }
        }
        if (effectStart) {
            cache->timeMS = SettingsMap.GetInt("TEXTCTRL_Shader_LeadIn", 0) * buffer.frameTimeInMs;
        }

        float oset = buffer.GetEffectTimeIntervalPosition();
        double timeRate = GetValueCurveDouble("Shader_Speed", 100, SettingsMap, oset, SHADER_SPEED_MIN, SHADER_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 100.0;
        cache->timeMS += buffer.frameTimeInMs * timeRate;
        // NOTE: buffer.needToInit is cleared only after a successful encode —
        // the base GL fallback relies on it to initialize its own state.

        double offsetX = GetValueCurveInt("Shader_Offset_X", 0, SettingsMap, oset, SHADER_OFFSET_X_MIN, SHADER_OFFSET_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 200.0 + 0.5;
        double offsetY = GetValueCurveInt("Shader_Offset_Y", 0, SettingsMap, oset, SHADER_OFFSET_Y_MIN, SHADER_OFFSET_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 200.0 + 0.5;
        double zoom = GetValueCurveInt("Shader_Zoom", 0, SettingsMap, oset, SHADER_ZOOM_MIN, SHADER_ZOOM_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1);
        if (zoom < 0) zoom = 1.0 - std::abs(zoom) / 100.0;
        else if (zoom > 0) zoom = 1.0 + (zoom * 9.0) / 100.0;
        else zoom = 1.0;

        // Compute all uniform values as floats; the reflected type decides how
        // each is marshalled at bind time (float bits vs int bits).
        std::unordered_map<std::string, std::array<float, 4>> vals;
        auto set1 = [&](const std::string& n, float a) { vals[n] = { a, 0, 0, 0 }; };
        auto set2 = [&](const std::string& n, float a, float b) { vals[n] = { a, b, 0, 0 }; };
        auto set4 = [&](const std::string& n, float a, float b, float c, float d) { vals[n] = { a, b, c, d }; };

        set2("RENDERSIZE", buffer.BufferWi, buffer.BufferHt);
        set2("XL_OFFSET", offsetX, offsetY);
        set1("XL_ZOOM", zoom);
        set1("XL_DURATION", (buffer.GetEndTimeMS() - buffer.GetStartTimeMS()) / 1000.0);
        set1("TIME", cache->timeMS / 1000.0);
        set1("TIMEDELTA", buffer.frameTimeInMs / 1000.f);
        {
            std::time_t nowt = std::time(nullptr);
            std::tm tmbuf;
            localtime_r(&nowt, &tmbuf);
            set4("DATE", tmbuf.tm_year + 1900, tmbuf.tm_mon + 1, tmbuf.tm_mday, tmbuf.tm_hour * 3600 + tmbuf.tm_min * 60 + tmbuf.tm_sec);
        }
        set1("NUMCOLORS", buffer.GetColorCount());
        set1("PASSINDEX", 0);
        set1("FRAMEINDEX", cache->timeMS / buffer.frameTimeInMs);
        set1("clearBuffer", SettingsMap.GetBool("CHECKBOX_OverlayBkg", false) ? 1.f : 0.f);
        set1("resetNow", (buffer.curPeriod == buffer.curEffStartPer) ? 1.f : 0.f);

        int colourIndex = 0;
        for (const auto& it : cache->config->GetParms()) {
            switch (it._type) {
            case ShaderParmType::SHADER_PARM_FLOAT:
                set1(it._name, GetValueCurveDouble(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_VALUECURVE), it._default * 100.0, SettingsMap, oset, it._min * 100.0, it._max * 100.0, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 100.0);
                break;
            case ShaderParmType::SHADER_PARM_POINT2D: {
                double x = GetValueCurveDouble(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_VALUECURVE) + "X", it._defaultPt.x * 100, SettingsMap, oset, it._minPt.x * 100, it._maxPt.x * 100, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 100.0;
                double y = GetValueCurveDouble(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_VALUECURVE) + "Y", it._defaultPt.y * 100, SettingsMap, oset, it._minPt.y * 100, it._maxPt.y * 100, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 100.0;
                set2(it._name, x, y);
                break;
            }
            case ShaderParmType::SHADER_PARM_BOOL:
                set1(it._name, SettingsMap.GetBool(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_CHECKBOX)) ? 1.f : 0.f);
                break;
            case ShaderParmType::SHADER_PARM_EVENT: {
                auto timingtrack = SettingsMap.Get(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_TIMING), "");
                EffectLayer* el = GetTiming(timingtrack, GetSequenceElements(buffer));
                bool b = false;
                if (el != nullptr) {
                    int ms = buffer.curPeriod * buffer.frameTimeInMs;
                    for (int j = 0; j < el->GetEffectCount(); j++) {
                        int ems = el->GetEffect(j)->GetStartTimeMS();
                        if (ems == ms) {
                            b = true;
                            break;
                        }
                        if (ems > ms) break;
                    }
                }
                set1(it._name, b ? 1.f : 0.f);
                break;
            }
            case ShaderParmType::SHADER_PARM_LONGCHOICE:
                set1(it._name, (float)it.EncodeChoice(SettingsMap[it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_CHOICE)]));
                break;
            case ShaderParmType::SHADER_PARM_LONG:
                set1(it._name, (float)GetValueCurveInt(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_VALUECURVE), it._default, SettingsMap, oset, it._min, it._max, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1));
                break;
            case ShaderParmType::SHADER_PARM_COLOUR: {
                xlColor c = buffer.palette.GetColor(colourIndex);
                colourIndex++;
                if (colourIndex > (int)buffer.GetColorCount()) colourIndex = 0;
                set4(it._name, c.red / 255.0, c.green / 255.0, c.blue / 255.0, 1.0);
                break;
            }
            default:
                break;
            }
        }

        // ---- encode into the RenderBuffer's own pixel MTLBuffer ----
        if (!cache->encode(buffer, vals, SettingsMap.GetBool("CHECKBOX_OverlayBkg", false))) {
            ShaderEffect::Render(eff, SettingsMap, buffer);
            return;
        }
        buffer.needToInit = false;

        // Debug probe: force GPU completion and inspect what actually landed in
        // buffer.pixels — separates "draw produces black" from "output never
        // reaches the pixels the engine reads".
        static const bool sProbe = getenv("XL_NATIVE_SHADER_DEBUG") != nullptr;
        if (sProbe && (buffer.curPeriod % 100) == 0) {
            GPURenderUtils::waitForRenderCompletion(&buffer);
            const uint8_t* px = (const uint8_t*)buffer.GetPixels();
            uint64_t sum = 0;
            const size_t n = std::min<size_t>((size_t)buffer.BufferWi * buffer.BufferHt * 4, 65536);
            for (size_t i = 0; i < n; i++) sum += px[i];
            fprintf(stderr, "NATIVE probe f=%d %s %dx%d TIME=%.2f pixsum(first64k)=%llu px0=(%u,%u,%u,%u)\n",
                    buffer.curPeriod, cache->shaderFile.c_str(), buffer.BufferWi, buffer.BufferHt,
                    cache->timeMS / 1000.0, (unsigned long long)sum, px[0], px[1], px[2], px[3]);
        }
    }
}
