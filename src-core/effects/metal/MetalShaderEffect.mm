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

// -----------------------------------------------------------------------
// MetalShaderEffect implementation
// -----------------------------------------------------------------------

MetalShaderEffect::MetalShaderEffect(int i) : ShaderEffect(i) {
}

MetalShaderEffect::~MetalShaderEffect() {
}

// -----------------------------------------------------------------------
// Native Metal render path
// -----------------------------------------------------------------------

namespace {

// Per-RenderBuffer cache for the native pipeline + config. Distinct slot from
// the base ShaderRenderCache (infoCache[id]).
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

// Command queue for the standalone path (GPU-rendering preference off, so no
// MetalRenderBufferComputeData exists): the frame is committed + waited on
// inline here instead of riding the render pipeline's per-buffer command
// buffer. One shared queue for all shader effects.
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
    bool failed = false;            // parse or translation/pipeline failure — latched
    xlColor failColor = xlYELLOW;   // red = file missing/unparseable, yellow = build failure

    void reset() {
        delete config;
        config = nullptr;
        built = false;
        failed = false;
        failColor = xlYELLOW;
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

    // Encode one frame into the RenderBuffer's pixel MTLBuffer (GPU compute on)
    // or into a private staging buffer read back to buffer.pixels (GPU compute
    // off — no MetalRenderBufferComputeData exists). Returns false if it can't
    // run this frame.
    bool encode(RenderBuffer& buffer, const UniformValues& vals, bool overlay) {
        static const bool dbg = getenv("XL_NATIVE_SHADER_DEBUG") != nullptr;
        const NSUInteger bpr = buffer.BufferWi * 4;
        MetalRenderBufferComputeData* rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
        id<MTLCommandBuffer> commandBuffer = nil;
        id<MTLBuffer> pixelBuffer = nil;
        if (rbcd != nullptr) {
            commandBuffer = rbcd->getCommandBuffer();
            if (commandBuffer == nil) {
                if (dbg) fprintf(stderr, "NATIVE encodefail f=%d %s: no command buffer\n", buffer.curPeriod, shaderFile.c_str());
                return false;
            }
            pixelBuffer = rbcd->getPixelBuffer();
            if (pixelBuffer == nil || pixelBuffer.length < bpr * buffer.BufferHt) {
                if (dbg) fprintf(stderr, "NATIVE encodefail f=%d %s: pixbuf len=%lu need=%lu\n", buffer.curPeriod, shaderFile.c_str(),
                                 (unsigned long)(pixelBuffer ? pixelBuffer.length : 0), (unsigned long)(bpr * buffer.BufferHt));
                rbcd->abortCommandBuffer();
                return false;
            }
        } else {
            commandBuffer = [standaloneCommandQueue() commandBuffer];
            if (commandBuffer == nil) {
                if (dbg) fprintf(stderr, "NATIVE encodefail f=%d %s: no standalone command buffer\n", buffer.curPeriod, shaderFile.c_str());
                return false;
            }
            if (stagingBuffer == nil || stagingBuffer.length < bpr * buffer.BufferHt) {
                stagingBuffer = [MetalComputeUtilities::INSTANCE.device newBufferWithLength:bpr * buffer.BufferHt
                                                                                    options:MTLResourceStorageModeShared];
            }
            if (stagingBuffer == nil) {
                if (dbg) fprintf(stderr, "NATIVE encodefail f=%d %s: no staging buffer\n", buffer.curPeriod, shaderFile.c_str());
                return false;
            }
            pixelBuffer = stagingBuffer;
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

        // Copy the rendered tiled texture into the pixel buffer. With rbcd the
        // framework commits + waits on commandBuffer; standalone we do it here
        // synchronously and read the result back into buffer.pixels.
        id<MTLBlitCommandEncoder> blit = [commandBuffer blitCommandEncoder];
        [blit copyFromTexture:outputTex sourceSlice:0 sourceLevel:0
                 sourceOrigin:MTLOriginMake(0, 0, 0)
                   sourceSize:MTLSizeMake(buffer.BufferWi, buffer.BufferHt, 1)
                     toBuffer:pixelBuffer destinationOffset:0
        destinationBytesPerRow:bpr destinationBytesPerImage:bpr * buffer.BufferHt];
        [blit endEncoding];
        if (rbcd == nullptr) {
            [commandBuffer commit];
            [commandBuffer waitUntilCompleted];
            memcpy(buffer.GetPixels(), stagingBuffer.contents, bpr * buffer.BufferHt);
        }
        return true;
    }

private:
    id<MTLRenderPipelineState> pso = nil;
    id<MTLSamplerState> sampler = nil;
    id<MTLBuffer> quadBuffer = nil;
    id<MTLTexture> inputTex = nil;
    id<MTLTexture> outputTex = nil;
    id<MTLTexture> audioTex = nil; // 128x1 R32F FFT/intensity, replaces inputTex for audio shaders
    id<MTLBuffer> stagingBuffer = nil; // standalone-path readback (no rbcd)
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
#if !TARGET_OS_IPHONE
    // macOS-desktop diagnostics/comparison fallback: XL_NO_NATIVE_SHADER=1
    // routes to the base OpenGL (CGL) path. iOS has no GL path at all.
    static const bool nativeEnabled = getenv("XL_NO_NATIVE_SHADER") == nullptr;
    if (!nativeEnabled) {
        ShaderEffect::Render(eff, SettingsMap, buffer);
        return;
    }
#endif
    const std::string shaderFile = SettingsMap.Get("0FILEPICKERCTRL_IFS", "");
    if (shaderFile.empty()) {
        buffer.Fill(xlRED); // no shader file configured — same as the base path
        return;
    }
    auto* cache = getNativeCache(id, buffer);
    // A new effect (or a different shader file) on the same buffer must not
    // reuse the previous effect's compiled pipeline/config — or its failed
    // flag. The infoCache slot is per (buffer, effect TYPE), not per instance,
    // so this reset must run BEFORE the failed check.
    if (cache->shaderFile != shaderFile) {
        cache->reset();
        cache->shaderFile = shaderFile;
    }
    if (cache->failed) {
        buffer.Fill(cache->failColor);
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
                cache->failColor = xlRED; // missing/unparseable — same as the base path
                buffer.Fill(cache->failColor);
                return;
            }
            cache->built = false;
            if (!cache->build(buffer)) {
                cache->failed = true;
                cache->failColor = xlYELLOW; // translation/pipeline failure — same as a GL compile failure
                buffer.Fill(cache->failColor);
                return;
            }
        }
        if (effectStart) {
            cache->timeMS = SettingsMap.GetInt("TEXTCTRL_Shader_LeadIn", 0) * buffer.frameTimeInMs;
        }

        float oset = buffer.GetEffectTimeIntervalPosition();
        double timeRate = GetValueCurveDouble("Shader_Speed", 100, SettingsMap, oset, SHADER_SPEED_MIN, SHADER_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 100.0;
        cache->timeMS += buffer.frameTimeInMs * timeRate;

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
            // Transient (no command buffer / undersized pixel buffer) — fill
            // this frame rather than latching failed.
            buffer.Fill(xlYELLOW);
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
