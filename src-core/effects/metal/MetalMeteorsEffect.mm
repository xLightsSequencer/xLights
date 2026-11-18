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
#include "MetalEffectDataTypes.h"

#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../MeteorsEffect.h"

#include <vector>
#include <algorithm>

class MetalMeteorsEffectData {
public:
    MetalMeteorsEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("MeteorsEffect");
        scatterFn = MetalComputeUtilities::INSTANCE.FindComputeFunction("MeteorsRadialScatter");
        resolveFn = MetalComputeUtilities::INSTANCE.FindComputeFunction("MeteorsRadialResolve");
    }

    bool canRender() { return fn != nil; }
    bool canRenderRadial() { return scatterFn != nil && resolveFn != nil; }

    bool RenderRadial(MetalMeteorsRadialData &mdata, const std::vector<MetalMeteorRadial> &parts, RenderBuffer &buffer) {
        @autoreleasepool {
            MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
            if (!rbcd) return false;

            id<MTLCommandBuffer> commandBuffer = rbcd->getCommandBuffer();
            if (commandBuffer == nil) return false;

            id<MTLBuffer> bufferResult = rbcd->getPixelBuffer();
            if (bufferResult == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }

            id<MTLDevice> device = scatterFn.device;
            NSUInteger pixels = (NSUInteger)mdata.width * (NSUInteger)mdata.height;
            id<MTLBuffer> partBuffer = [device newBufferWithBytes:parts.data()
                                                           length:parts.size() * sizeof(MetalMeteorRadial)
                                                          options:MTLResourceStorageModeShared];
            // The key buffer is scratch that only the GPU ever reads or writes,
            // so it never needs to come back across the bus.
            id<MTLBuffer> keyBuffer = [device newBufferWithLength:pixels * sizeof(uint32_t)
                                                          options:MTLResourceStorageModePrivate];
            if (partBuffer == nil || keyBuffer == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }

            id<MTLBlitCommandEncoder> blit = [commandBuffer blitCommandEncoder];
            if (blit == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            [blit fillBuffer:keyBuffer range:NSMakeRange(0, pixels * sizeof(uint32_t)) value:0];
            [blit endEncoding];

            bool encoded = rbcd->encodeEffect(commandBuffer, scatterFn, @"MeteorsRadialScatter", [&](id<MTLComputeCommandEncoder> enc) {
                [enc setBytes:&mdata length:sizeof(mdata) atIndex:0];
                [enc setBuffer:partBuffer offset:0 atIndex:1];
                [enc setBuffer:keyBuffer offset:0 atIndex:2];
                MetalRenderBufferComputeData::dispatch1D(enc, scatterFn, (NSInteger)parts.size() * (NSInteger)mdata.stride);
            });
            if (!encoded) {
                return false;
            }
            encoded = rbcd->encodeEffect(commandBuffer, resolveFn, @"MeteorsRadialResolve", [&](id<MTLComputeCommandEncoder> enc2) {
                [enc2 setBytes:&mdata length:sizeof(mdata) atIndex:0];
                [enc2 setBuffer:bufferResult offset:0 atIndex:1];
                [enc2 setBuffer:partBuffer offset:0 atIndex:2];
                [enc2 setBuffer:keyBuffer offset:0 atIndex:3];
                MetalRenderBufferComputeData::dispatch1D(enc2, resolveFn, (NSInteger)pixels);
            });
            if (!encoded) {
                return false;
            }
        }
        return true;
    }

    bool Render(MetalMeteorsData &mdata, const std::vector<MetalMeteorParticle> &parts,
                const std::vector<int> &lineItems, const std::vector<int> &lineStart, RenderBuffer &buffer) {
        @autoreleasepool {
            MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
            if (!rbcd) return false;

            id<MTLCommandBuffer> commandBuffer = rbcd->getCommandBuffer();
            if (commandBuffer == nil) return false;

            id<MTLBuffer> bufferResult = rbcd->getPixelBuffer();
            if (bufferResult == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }

            // The kernel needs valid particle and bucket buffers even when there are no
            // meteors (an icicle frame may still draw its background), so allocate at
            // least one element of each. lineStart always has lineCount+1 entries.
            MetalMeteorParticle zero {};
            const void *src = parts.empty() ? (const void *)&zero : (const void *)parts.data();
            NSUInteger len = std::max((size_t)1, parts.size()) * sizeof(MetalMeteorParticle);
            id<MTLDevice> device = fn.device;
            id<MTLBuffer> partBuffer = [device newBufferWithBytes:src length:len options:MTLResourceStorageModeShared];
            if (partBuffer == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            int noItems = 0;
            const void *itemSrc = lineItems.empty() ? (const void *)&noItems : (const void *)lineItems.data();
            NSUInteger itemLen = std::max((size_t)1, lineItems.size()) * sizeof(int);
            id<MTLBuffer> itemBuffer = [device newBufferWithBytes:itemSrc length:itemLen options:MTLResourceStorageModeShared];
            id<MTLBuffer> startBuffer = [device newBufferWithBytes:lineStart.data()
                                                            length:lineStart.size() * sizeof(int)
                                                           options:MTLResourceStorageModeShared];
            if (itemBuffer == nil || startBuffer == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }

            bool encoded = rbcd->encodeEffect(commandBuffer, fn, @"MeteorsEffect", [&](id<MTLComputeCommandEncoder> computeEncoder) {
                [computeEncoder setBytes:&mdata length:sizeof(mdata) atIndex:0];
                [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];
                [computeEncoder setBuffer:partBuffer offset:0 atIndex:2];
                [computeEncoder setBuffer:itemBuffer offset:0 atIndex:3];
                [computeEncoder setBuffer:startBuffer offset:0 atIndex:4];

                NSInteger pixelCount = mdata.width * mdata.height;
                MetalRenderBufferComputeData::dispatch1D(computeEncoder, fn, pixelCount);
            });
            if (!encoded) {
                return false;
            }
        }
        return true;
    }

private:
    id<MTLComputePipelineState> fn = nil;
    id<MTLComputePipelineState> scatterFn = nil;
    id<MTLComputePipelineState> resolveFn = nil;
};


MetalMeteorsEffect::MetalMeteorsEffect(int i) : MeteorsEffect(i) {
    data = new MetalMeteorsEffectData();
}
MetalMeteorsEffect::~MetalMeteorsEffect() {
    if (data) { delete data; }
}

void MetalMeteorsEffect::GatherMeteors(RenderBuffer& buffer, const MeteorsGatherParams& params, const std::vector<MeteorSnapshot>& parts) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || (buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold) {
        MeteorsEffect::GatherMeteors(buffer, params, parts);
        return;
    }

    MetalMeteorsData mdata;
    mdata.width = buffer.BufferWi;
    mdata.height = buffer.BufferHt;
    mdata.mode = params.mode;
    mdata.direction = params.direction;
    mdata.tailLength = params.tailLength;
    mdata.colorScheme = params.colorScheme;
    mdata.allowAlpha = params.allowAlpha;
    mdata.numMeteors = (int)parts.size();
    mdata.wantBkg = params.wantBkg;
    mdata.frameSeed = params.frameSeed;

    std::vector<MetalMeteorParticle> mp(parts.size());
    for (size_t i = 0; i < parts.size(); i++) {
        mp[i].a = parts[i].a;
        mp[i].base = parts[i].base;
        mp[i].h = parts[i].h;
        mp[i].hue = (float)parts[i].hue;
        mp[i].sat = (float)parts[i].sat;
        mp[i].val = (float)parts[i].val;
    }

    const int lineCount = (params.mode == 1) ? buffer.BufferHt : buffer.BufferWi;
    std::vector<int> lineStart;
    std::vector<int> lineItems;
    BucketMeteorsByLine(parts, lineCount, lineStart, lineItems);

    if (!data->Render(mdata, mp, lineItems, lineStart, buffer)) {
        MeteorsEffect::GatherMeteors(buffer, params, parts);
    }
}

void MetalMeteorsEffect::DrawRadialSnapshot(RenderBuffer& buffer, const MeteorsRadialFrameState& fs) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRenderRadial() || fs.meteors.empty() || buffer.dmx_buffer
        || (buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold) {
        MeteorsEffect::DrawRadialSnapshot(buffer, fs);
        return;
    }

    MetalMeteorsRadialData mdata;
    mdata.width = buffer.BufferWi;
    mdata.height = buffer.BufferHt;
    mdata.implode = fs.implode ? 1 : 0;
    mdata.tailLength = fs.tailLength;
    mdata.stride = fs.tailLength + 1;
    mdata.colorScheme = fs.colorScheme;
    mdata.allowAlpha = buffer.allowAlpha ? 1 : 0;
    mdata.fadeWithDistance = fs.fadeWithDistance ? 1 : 0;
    mdata.centerX = fs.centerX;
    mdata.centerY = fs.centerY;
    mdata.maxdiag = fs.maxdiag;
    mdata.numMeteors = (int)fs.meteors.size();
    mdata.frameSeed = buffer.hashRandomFrameSeed();

    // The key is n*stride + ph + 1 in a uint32; a buffer that could overflow it
    // would need billions of trail steps, but bail to the CPU rather than wrap.
    if ((int64_t)fs.meteors.size() * mdata.stride >= (int64_t)UINT32_MAX) {
        MeteorsEffect::DrawRadialSnapshot(buffer, fs);
        return;
    }

    std::vector<int> cuts;
    ComputeRadialCuts(fs, cuts);

    std::vector<MetalMeteorRadial> mp(fs.meteors.size());
    for (size_t i = 0; i < fs.meteors.size(); i++) {
        mp[i].x = fs.meteors[i].x;
        mp[i].y = fs.meteors[i].y;
        mp[i].dx = fs.meteors[i].dx;
        mp[i].dy = fs.meteors[i].dy;
        mp[i].hue = fs.meteors[i].hsv.hue;
        mp[i].sat = fs.meteors[i].hsv.saturation;
        mp[i].val = fs.meteors[i].hsv.value;
        mp[i].cut = cuts[i];
    }

    if (!data->RenderRadial(mdata, mp, buffer)) {
        MeteorsEffect::DrawRadialSnapshot(buffer, fs);
    }
}
