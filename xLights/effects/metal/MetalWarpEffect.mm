#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../RenderBuffer.h"
#include "DissolveTransitionPattern.h"
#include <array>

class MetalWarpEffectData {
public:
    MetalWarpEffectData() {
        for (auto &f : functions) {
            f = nil;
        }
        functions[WarpEffect::WarpType::MIRROR] = MetalComputeUtilities::INSTANCE.FindComputeFunction("WarpEffectMirror");
        functions[WarpEffect::WarpType::COPY] = MetalComputeUtilities::INSTANCE.FindComputeFunction("WarpEffectCopy");
        functions[WarpEffect::WarpType::SAMPLE_ON] = MetalComputeUtilities::INSTANCE.FindComputeFunction("WarpEffectSampleOn");
        functions[WarpEffect::WarpType::WAVY] = MetalComputeUtilities::INSTANCE.FindComputeFunction("WarpEffectWavy");
        functions[WarpEffect::WarpType::WATER_DROPS] = MetalComputeUtilities::INSTANCE.FindComputeFunction("WarpEffectWaterDrops");
        functions[WarpEffect::WarpType::SINGLE_WATER_DROP] = MetalComputeUtilities::INSTANCE.FindComputeFunction("WarpEffectSingleWaterDrop");
        functions[WarpEffect::WarpType::DISSOLVE] = MetalComputeUtilities::INSTANCE.FindComputeFunction("WarpEffectDissolve");
        functions[WarpEffect::WarpType::RIPPLE] = MetalComputeUtilities::INSTANCE.FindComputeFunction("WarpEffectRipple");
        functions[WarpEffect::WarpType::DROP] = MetalComputeUtilities::INSTANCE.FindComputeFunction("WarpEffectDrop");
        functions[WarpEffect::WarpType::CIRCLE_REVEAL] = MetalComputeUtilities::INSTANCE.FindComputeFunction("WarpEffectCircleReveal");
        functions[WarpEffect::WarpType::CIRCULAR_SWIRL] = MetalComputeUtilities::INSTANCE.FindComputeFunction("WarpEffectCircleSwirl");
        functions[WarpEffect::WarpType::BANDED_SWIRL] = MetalComputeUtilities::INSTANCE.FindComputeFunction("WarpEffectBandedSwirl");

        int bufferSize = DissolvePatternWidth * DissolvePatternHeight;
        dissolvePatternBuffer = [[MetalComputeUtilities::INSTANCE.device
                                       newBufferWithBytes:DissolveTransitonPattern
                                                   length:bufferSize
                                                  options:MTLResourceStorageModeShared] retain];

        [dissolvePatternBuffer setLabel:@"DissolveTransitonPattern"];
    }
    ~MetalWarpEffectData() {
        if (dissolvePatternBuffer != nil) {
            [dissolvePatternBuffer release];
        }
        for (auto &f : functions) {
            if (f != nil) {
                [f release];
            }
        }
    }
    bool requiresBufferCopy(WarpEffect::WarpType st) {
        switch (st) {
            case WarpEffect::WarpType::COPY:
            case WarpEffect::WarpType::WAVY:
            case WarpEffect::WarpType::WATER_DROPS:
            case WarpEffect::WarpType::SINGLE_WATER_DROP:
            case WarpEffect::WarpType::RIPPLE:
            case WarpEffect::WarpType::DROP:
            case WarpEffect::WarpType::CIRCULAR_SWIRL:
            case WarpEffect::WarpType::BANDED_SWIRL:
                return true;
            default:
                return false;
        }
    }
    bool requiresDissolvePattern(WarpEffect::WarpType st) {
        switch (st) {
            case WarpEffect::WarpType::WAVY:
            case WarpEffect::WarpType::DISSOLVE:
            case WarpEffect::WarpType::DROP:
                return true;
            default:
                return false;
        }
    }

    bool canRenderStyle(WarpEffect::WarpType st) {
        int style = st;
        return style >= 0 && style < functions.size() && functions[style] != nil;
    }

    bool Render(WarpEffect::WarpType style, WarpData &data, RenderBuffer &buffer) {
        
        @autoreleasepool {
            MetalRenderBufferComputeData * rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
            id<MTLBuffer> bufferResult = rbcd->getPixelBuffer();
            if (bufferResult == nil) {
                return false;
            }

            id<MTLCommandBuffer> commandBuffer = rbcd->getCommandBuffer();
            if (commandBuffer == nil) {
                return false;
            }
            id<MTLBuffer> bufferCopy = nil;
            if (requiresBufferCopy(style)) {
                bufferCopy = rbcd->getPixelBufferCopy();
                id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
                [blitCommandEncoder setLabel:@"CopyDataToCopyBuffer"];
                [blitCommandEncoder copyFromBuffer:bufferResult
                                      sourceOffset:0
                                          toBuffer:bufferCopy
                                 destinationOffset:0
                                              size:(data.width*data.height*4)];
                [blitCommandEncoder endEncoding];
            }
            
            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            if (computeEncoder == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            [computeEncoder setLabel:@"WarpEffect"];
            [computeEncoder setComputePipelineState:functions[style]];

            NSInteger dataSize = sizeof(data);
            [computeEncoder setBytes:&data length:dataSize atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];
            
            int curIdx = 2;
            if (bufferCopy != nil) {
                [computeEncoder setBuffer:bufferCopy offset:0 atIndex:curIdx];
                ++curIdx;
            }
            if (requiresDissolvePattern(style)) {
                [computeEncoder setBuffer:dissolvePatternBuffer offset:0 atIndex:curIdx];
                ++curIdx;
            }
            
            int w = functions[style].threadExecutionWidth;
            int h = functions[style].maxTotalThreadsPerThreadgroup / w;
            MTLSize threadsPerThreadgroup = MTLSizeMake(w, h, 1);
            MTLSize threadsPerGrid = MTLSizeMake(data.width, data.height, 1);
            
            [computeEncoder dispatchThreads:threadsPerGrid
                      threadsPerThreadgroup:threadsPerThreadgroup];

            [computeEncoder endEncoding];
        }
        return true;
    }
    id<MTLBuffer> dissolvePatternBuffer = nil;
    std::array<id<MTLComputePipelineState>, WarpEffect::WarpType::COUNT_WARP_STYLES> functions;
};

MetalWarpEffect::MetalWarpEffect(int i) : WarpEffect(i) {
    data = new MetalWarpEffectData();
}
MetalWarpEffect::~MetalWarpEffect() {
    if (data) {
        delete data;
    }
}

static inline double interpolate( double x, double loIn, double loOut, double hiIn, double hiOut) {
   return ( loIn != hiIn )
      ? ( loOut + (hiOut - loOut) * ( (x-loIn)/(hiIn-loIn) ) )
      : ( (loOut + hiOut) / 2 );
}

void MetalWarpEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData * rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    std::string warpType = SettingsMap.Get( "CHOICE_Warp_Type", "water drops" );
    WarpEffect::WarpType Style = mapWarpType(warpType);

    //currently just  Styles 1-5 are GPU enabled, if smaller buffer, overhead of prep for GPU will be higher than benefit
    if (rbcd == nullptr || !data->canRenderStyle(Style) || ((buffer.BufferWi * buffer.BufferHt) < 1024)) {
        WarpEffect::Render(effect, SettingsMap, buffer);
        return;
    }


    WarpData wdata;
    wdata.width = buffer.BufferWi;
    wdata.height = buffer.BufferHt;
    
    float progress = buffer.GetEffectTimeIntervalPosition(1.f);
    std::string warpTreatment = SettingsMap.Get( "CHOICE_Warp_Treatment_APPLYLAST", "constant");
    float xPercentage = GetValueCurveInt("Warp_X", 0, SettingsMap, progress, 0, 100,
                                       buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float yPercentage = GetValueCurveInt("Warp_Y", 0, SettingsMap, progress, 0, 100,
                                       buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float x = 0.01 * xPercentage;
    float y = 0.01 * yPercentage;

    wdata.xPos = std::round((float)buffer.BufferWi - 1) * x;
    wdata.yPos = std::round((float)buffer.BufferHt - 1) * y;
    wdata.progress = progress;
    wdata.speed = SettingsMap.GetFloat("TEXTCTRL_Warp_Speed", 20.0);
    wdata.frequency = SettingsMap.GetFloat("TEXTCTRL_Warp_Frequency", 20.0);
    wdata.direction = warpTreatment == "out";
    
    //adjust params based on type (if needed)
    switch (Style) {
        case WarpEffect::WarpType::WATER_DROPS:
        case WarpEffect::WarpType::SAMPLE_ON:
        case WarpEffect::WarpType::MIRROR:
        case WarpEffect::WarpType::COPY:
            break;
        case WarpEffect::WarpType::WAVY:
            wdata.speed = interpolate(wdata.speed, 0.0, 0.5, 40.0, 5.0);
            break;
        case WarpEffect::WarpType::SINGLE_WATER_DROP:
            {
                float cycleCount = SettingsMap.GetFloat("TEXTCTRL_Warp_Cycle_Count", 1.0);
                float intervalLen = 1.f / cycleCount;
                float scaledProgress = progress / intervalLen;
                float intervalProgress, intervalIndex;
                intervalProgress = std::modf(scaledProgress, &intervalIndex);
                float interpolatedProgress = interpolate(intervalProgress, 0.0, 0.20, 1.0, 0.45 );
                wdata.progress = interpolatedProgress;
            }
            break;
        default:
            if (warpTreatment == "constant") {
                float cycleCount = SettingsMap.GetFloat("TEXTCTRL_Warp_Cycle_Count", 1.0);
                float intervalLen = 1.f / (2 * cycleCount );
                float scaledProgress = progress / intervalLen;
                float intervalProgress, intervalIndex;
                intervalProgress = std::modf( scaledProgress, &intervalIndex );
                if (int(intervalIndex) % 2) {
                    intervalProgress = 1.f - intervalProgress;
                }
                wdata.progress = intervalProgress;
                if (Style == WarpEffect::WarpType::CIRCULAR_SWIRL || Style == WarpEffect::WarpType::DROP) {
                    wdata.progress = 1. - wdata.progress;
                }
            } else {
                if (Style == WarpEffect::WarpType::DROP && warpTreatment == "in") {
                    wdata.progress = 1. - wdata.progress;
                }
            }
            if (Style == WarpEffect::WarpType::CIRCULAR_SWIRL) {
                wdata.speed = interpolate(wdata.speed, 0.0, 1.0, 40.0, 9.0);
                if (warpTreatment == "in") {
                    wdata.progress = 1. - wdata.progress;
                }
            }
            break;
    }
    if (data->Render(Style, wdata, buffer)) {
        return;
    }
    WarpEffect::Render(effect, SettingsMap, buffer);
}
