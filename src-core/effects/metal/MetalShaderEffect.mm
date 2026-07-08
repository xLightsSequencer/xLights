/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Metal backend of the shared SPIRVShaderEffect: translates the (already
// source-transformed) ISF fragment GLSL to MSL via MetalShaderTranslator,
// builds a render pipeline (cached process-wide by fragment source), and
// encodes a fullscreen quad into the RenderBuffer's pixel MTLBuffer. All
// lifecycle/uniform/parm/audio logic lives in SPIRVShaderEffect.
#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalShaderTranslator.h"

#include "../../render/RenderBuffer.h"
#include "../ShaderEffect.h"

#import <Metal/Metal.h>
#include <TargetConditionals.h>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>

namespace {

// Process-wide program cache keyed by the transformed fragment source (the
// vertex shader is fixed). Translation + MSL compile + pipeline build cost
// ~40ms per shader; without this every (effect x buffer) pays it. Mirrors the
// GL path's source-keyed shaderMap. Failures are cached too.
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

// Command queue for the standalone path (GPU-rendering preference off, so no
// MetalRenderBufferComputeData exists): the frame is committed + waited on
// inline instead of riding the render pipeline's per-buffer command buffer.
static id<MTLCommandQueue> standaloneCommandQueue() {
    static id<MTLCommandQueue> queue = []() {
        id<MTLCommandQueue> q = [MetalComputeUtilities::INSTANCE.device newCommandQueue];
        [q setLabel:@"NativeShaderEffectStandalone"];
        return q;
    }();
    return queue;
}

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

// Metal-specific per-buffer state. Kept off MetalShaderEffect because that
// class's inherited `int id` data member shadows the ObjC `id` type inside its
// methods (as it would here otherwise).
class MetalShaderNativeCache : public SPIRVShaderEffect::CacheBase {
public:
    MetalShaderNativeCache() {}
    virtual ~MetalShaderNativeCache() {} // ARC releases the ObjC members

    virtual void platformReset() override {
        pso = nil;
        outputTex = nil;
        inputTex = nil;
        audioTex = nil;
        stagingBuffer = nil;
    }

    // Fetch (or build once, process-wide) the translated program, then create
    // the per-buffer-size resources.
    bool build(RenderBuffer& buffer) {
        id<MTLDevice> device = MetalComputeUtilities::INSTANCE.device;
        CachedShaderProgram prog;
        {
            std::lock_guard<std::mutex> lock(sProgramCacheMutex);
            auto& cache = programCache();
            auto it = cache.find(transformedSource);
            if (it == cache.end()) {
                it = cache.emplace(transformedSource, buildShaderProgram(transformedSource, config->GetFilename())).first;
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
        // after each frame. Kept across frames so overlay (loadAction Load)
        // reads the prior frame.
        MTLTextureDescriptor* otd = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                      width:buffer.BufferWi height:buffer.BufferHt mipmapped:NO];
        otd.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
        otd.storageMode = MTLStorageModePrivate;
        outputTex = [device newTextureWithDescriptor:otd];
        return true;
    }

    // Encode one frame. When the render pipeline's per-buffer command buffer is
    // available the frame rides it (committed + synced by the engine); with GPU
    // compute disabled it renders standalone on a private queue, synchronously.
    bool encode(RenderBuffer& buffer, const SPIRVShaderEffect::UniformValues& vals,
                SPIRVShaderEffect::InputKind kind, const float* audio128, bool overlay) {
        static const bool dbg = getenv("XL_NATIVE_SHADER_DEBUG") != nullptr;
        const NSUInteger bpr = buffer.BufferWi * 4;

        MetalRenderBufferComputeData* rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
        id<MTLCommandBuffer> commandBuffer = nil;
        id<MTLBuffer> pixelBuffer = nil;
        bool standalone = false;
        if (rbcd != nullptr) {
            commandBuffer = rbcd->getCommandBuffer();
            pixelBuffer = commandBuffer ? rbcd->getPixelBuffer() : nil;
        }
        if (pixelBuffer == nil || pixelBuffer.length < bpr * buffer.BufferHt) {
            // GPU compute disabled (no rbcd / pixel buffer): standalone path.
            standalone = true;
            commandBuffer = [standaloneCommandQueue() commandBuffer];
            if (commandBuffer == nil) return false;
            if (!stagingBuffer || stagingBuffer.length < bpr * buffer.BufferHt) {
                stagingBuffer = [MetalComputeUtilities::INSTANCE.device newBufferWithLength:bpr * buffer.BufferHt
                                                                                    options:MTLResourceStorageModeShared];
            }
            if (stagingBuffer == nil) return false;
        }

        const bool usesTexture = fsInfo.samplerTexture >= 0;
        const bool audioMode = (kind == SPIRVShaderEffect::InputKind::Audio) && audioTex != nil;
        if (usesTexture && audioMode) {
            [audioTex replaceRegion:MTLRegionMake2D(0, 0, 128, 1)
                        mipmapLevel:0 withBytes:audio128 bytesPerRow:128 * 4];
        } else if (usesTexture) {
            // Feedback: upload a stable snapshot of the prior contents (the pixel
            // buffer, from last frame's blit) into the input texture.
            [inputTex replaceRegion:MTLRegionMake2D(0, 0, buffer.BufferWi, buffer.BufferHt)
                        mipmapLevel:0 withBytes:buffer.GetPixels() bytesPerRow:bpr];
        }

        if (dbg && (buffer.curPeriod % 100) == 0) {
            for (const auto& u : fsInfo.uniforms) {
                auto vit = vals.find(u.name);
                fprintf(stderr, "NATIVE bind f=%d %s: %s@%u = %s%.4f\n", buffer.curPeriod, shaderFile.c_str(),
                        u.name.c_str(), u.index, vit == vals.end() ? "MISSING " : "",
                        vit == vals.end() ? 0.0 : vit->second[0]);
            }
        }

        MTLRenderPassDescriptor* rp = [MTLRenderPassDescriptor renderPassDescriptor];
        rp.colorAttachments[0].texture = outputTex;
        rp.colorAttachments[0].loadAction = overlay ? MTLLoadActionLoad : MTLLoadActionClear;
        rp.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0);
        rp.colorAttachments[0].storeAction = MTLStoreActionStore;
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

        // Copy the rendered tiled texture into the pixel data (= buffer.pixels).
        id<MTLBuffer> dst = standalone ? stagingBuffer : pixelBuffer;
        id<MTLBlitCommandEncoder> blit = [commandBuffer blitCommandEncoder];
        [blit copyFromTexture:outputTex sourceSlice:0 sourceLevel:0
                 sourceOrigin:MTLOriginMake(0, 0, 0)
                   sourceSize:MTLSizeMake(buffer.BufferWi, buffer.BufferHt, 1)
                     toBuffer:dst destinationOffset:0
        destinationBytesPerRow:bpr destinationBytesPerImage:bpr * buffer.BufferHt];
        [blit endEncoding];

        if (standalone) {
            [commandBuffer commit];
            [commandBuffer waitUntilCompleted];
            std::memcpy(buffer.GetPixels(), stagingBuffer.contents, (size_t)bpr * buffer.BufferHt);
        }
        // Otherwise the render pipeline commits commandBuffer and syncs the
        // pixel buffer (which is buffer.pixels) via waitForRenderCompletion.
        return true;
    }

private:
    id<MTLRenderPipelineState> pso = nil;
    id<MTLSamplerState> sampler = nil;
    id<MTLBuffer> quadBuffer = nil;
    id<MTLTexture> inputTex = nil;
    id<MTLTexture> outputTex = nil;
    id<MTLTexture> audioTex = nil;
    id<MTLBuffer> stagingBuffer = nil;
    ShaderTranslate::ShaderStageInfo vsInfo;
    ShaderTranslate::ShaderStageInfo fsInfo;

    static void bindStage(id<MTLRenderCommandEncoder> enc, const ShaderTranslate::ShaderStageInfo& info,
                          const SPIRVShaderEffect::UniformValues& vals, bool vertex) {
        for (const auto& u : info.uniforms) {
            auto vit = vals.find(u.name);
            if (vit == vals.end()) continue;
            uint32_t tmp[4] = { 0, 0, 0, 0 };
            for (int i = 0; i < u.vecSize && i < 4; i++) {
                if (u.isFloat) {
                    float f = vit->second[i];
                    memcpy(&tmp[i], &f, 4);
                } else {
                    int iv = (int)lroundf(vit->second[i]);
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

} // namespace

// -----------------------------------------------------------------------
// MetalShaderEffect — thin Metal backend of SPIRVShaderEffect
// -----------------------------------------------------------------------

MetalShaderEffect::MetalShaderEffect(int i) :
    SPIRVShaderEffect(i) {
}

MetalShaderEffect::~MetalShaderEffect() {
}

bool MetalShaderEffect::nativeAvailable() const {
    return MetalComputeUtilities::INSTANCE.device != nil;
}

SPIRVShaderEffect::CacheBase* MetalShaderEffect::newCache() const {
    return new MetalShaderNativeCache();
}

bool MetalShaderEffect::nativeBuild(CacheBase* cache, RenderBuffer& buffer) {
    return static_cast<MetalShaderNativeCache*>(cache)->build(buffer);
}

bool MetalShaderEffect::nativeEncode(CacheBase* cache, RenderBuffer& buffer,
                                     const UniformValues& vals, InputKind kind,
                                     const float* audio128) {
    // Read the overlay flag out of the computed uniforms so the encode
    // signature stays platform-neutral (clearBuffer mirrors OverlayBkg).
    auto it = vals.find("clearBuffer");
    const bool overlay = (it != vals.end()) && it->second[0] != 0.f;
    return static_cast<MetalShaderNativeCache*>(cache)->encode(buffer, vals, kind, audio128, overlay);
}
