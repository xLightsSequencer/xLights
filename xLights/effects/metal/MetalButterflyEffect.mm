
#include "MetalComputeUtilities.hpp"
#include "MetalButterflyEffect.hpp"
#include "ButterflyTypes.h"

#include "../../RenderBuffer.h"


class MetalButterflyEffectData {
public:
    MetalButterflyEffectData() {
        function = MetalComputeUtilities::INSTANCE.FindComputeFunction("ButterflyEffect");
    }
    ~MetalButterflyEffectData() {
        function = nil;
    }


    uint32_t highestPowerof2(uint32_t x) {
        // check for the set bits
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;

        // Then we remove all but the top bit by xor'ing the
        // string of 1's with that string of 1's shifted one to
        // the left, and we end up with just the one top bit
        // followed by 0's.
        return x ^ (x >> 1);
    }

    bool Render(ButterflyData &data, RenderBuffer &buffer) {
        if (function == nil) {
            return false;
        }
        @autoreleasepool {
            MetalRenderBufferComputeData * rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);

            id<MTLCommandBuffer> commandBuffer = rbcd->getCommandBuffer();
            if (commandBuffer == nil) {
                return false;
            }
            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            if (computeEncoder == nil) {
                commandBuffer = nil;
                return false;
            }
            [computeEncoder setComputePipelineState:function];

            NSInteger dataSize = sizeof(data);
            [computeEncoder setBytes:&data length:dataSize atIndex:0];

            
            id<MTLBuffer> bufferResult = rbcd->getPixelBuffer();
            if (bufferResult == nil) {
                computeEncoder = nil;
                commandBuffer = nil;
                return false;
            }

            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];

            MTLSize gridSize = MTLSizeMake(data.width, data.height, 1);
            int maxtgs = function.maxTotalThreadsPerThreadgroup;
            int tgs = highestPowerof2(data.width);
            if (tgs > maxtgs || tgs < 1) {
                tgs = maxtgs;
            }
            MTLSize threadgroupSize = MTLSizeMake(tgs, 1, 1);
            [computeEncoder dispatchThreadgroups:gridSize
                      threadsPerThreadgroup:threadgroupSize];

            [computeEncoder endEncoding];
            [commandBuffer commit];
        }
        return true;
    }

    id<MTLComputePipelineState> function;
};

MetalButterflyEffect::MetalButterflyEffect(int i) : ButterflyEffect(i) {
    data = new MetalButterflyEffectData();
}
MetalButterflyEffect::~MetalButterflyEffect() {
    if (data) {
        delete data;
    }
}


void MetalButterflyEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData * rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    const int Style = SettingsMap.GetInt("SLIDER_Butterfly_Style", 1);

    //currently just  Style 1 is GPU enabled
    if (rbcd == nullptr || Style != 1) {
        ButterflyEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float oset = buffer.GetEffectTimeIntervalPosition();
    const int Chunks = GetValueCurveInt("Butterfly_Chunks", 1, SettingsMap, oset, BUTTERFLY_CHUNKS_MIN, BUTTERFLY_CHUNKS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int Skip = GetValueCurveInt("Butterfly_Skip", 2, SettingsMap, oset, BUTTERFLY_SKIP_MIN, BUTTERFLY_SKIP_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int butterFlySpeed = GetValueCurveInt("Butterfly_Speed", 10, SettingsMap, oset, BUTTERFLY_SPEED_MIN, BUTTERFLY_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    std::string colors = SettingsMap["CHOICE_Butterfly_Colors"];
    int ColorScheme = (colors == "Palette") ? 1 : 0;

    int ButterflyDirection = SettingsMap["CHOICE_Butterfly_Direction"] == "Reverse" ? 1 : 0;

    //const int maxframe=buffer.BufferHt*2;
    const int curState = (buffer.curPeriod - buffer.curEffStartPer) * butterFlySpeed * buffer.frameTimeInMs / 50;
    //const int frame=(buffer.BufferHt * curState / 200)%maxframe;
    //const size_t colorcnt=buffer.GetColorCount();
    const float offset = (ButterflyDirection==1 ? -1.0 : 1.0) * float(curState)/200.0f;
    //const int xc=buffer.BufferWi/2;
    //const int yc=buffer.BufferHt/2;
    //int block = buffer.BufferHt * buffer.BufferWi > 100 ? 1 : -1;

    ButterflyData rdata;
    rdata.width = buffer.BufferWi;
    rdata.height = buffer.BufferHt;
    rdata.numColors = buffer.palette.Size();
    rdata.offset = offset;
    rdata.chunks = Chunks;
    rdata.skip = Skip;
    rdata.colorScheme = ColorScheme;
    for (int x = 0; x < rdata.numColors; x++) {
        rdata.colors[x] = buffer.palette.GetColor(x).asChar4();
    }


    if (!data->Render(rdata, buffer)) {
        ButterflyEffect::Render(effect, SettingsMap, buffer);
    }
}
