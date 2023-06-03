
#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../RenderBuffer.h"

#include <array>

class MetalPlasmaEffectData {
public:
    MetalPlasmaEffectData() {
        for (auto &f : functions) {
            f = nil;
        }
        functions[0] = MetalComputeUtilities::INSTANCE.FindComputeFunction("PlasmaEffectStyle0");
        functions[1] = MetalComputeUtilities::INSTANCE.FindComputeFunction("PlasmaEffectStyle1");
        functions[2] = MetalComputeUtilities::INSTANCE.FindComputeFunction("PlasmaEffectStyle2");
        functions[3] = MetalComputeUtilities::INSTANCE.FindComputeFunction("PlasmaEffectStyle3");
        functions[4] = MetalComputeUtilities::INSTANCE.FindComputeFunction("PlasmaEffectStyle4");
        //functions[5] = MetalComputeUtilities::INSTANCE.FindComputeFunction("PlasmaEffectStyle5");
    }
    ~MetalPlasmaEffectData() {
        for (auto &f : functions) {
            if (f != nil) {
                [f release];
            }
        }
    }
    bool canRenderStyle(int style) {
        return style < functions.size() && functions[style] != nil;
    }

    bool Render(int ColorScheme, PlasmaData &data, RenderBuffer &buffer) {
        @autoreleasepool {
            MetalRenderBufferComputeData * rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);

            id<MTLCommandBuffer> commandBuffer = rbcd->getCommandBuffer();
            if (commandBuffer == nil) {
                return false;
            }
            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            if (computeEncoder == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            [computeEncoder setLabel:@"PlasmaEffect"];
            [computeEncoder setComputePipelineState:functions[ColorScheme]];

            NSInteger dataSize = sizeof(data);
            [computeEncoder setBytes:&data length:dataSize atIndex:0];

            
            id<MTLBuffer> bufferResult = rbcd->getPixelBuffer();
            if (bufferResult == nil) {
                computeEncoder = nil;
                rbcd->abortCommandBuffer();
                return false;
            }

            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];

            NSInteger maxThreads = functions[ColorScheme].maxTotalThreadsPerThreadgroup;
            dataSize = data.width * data.height;
            NSInteger threads = std::min(dataSize, maxThreads);
            MTLSize gridSize = MTLSizeMake(dataSize, 1, 1);
            MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);

            [computeEncoder dispatchThreads:gridSize
                      threadsPerThreadgroup:threadsPerThreadgroup];

            [computeEncoder endEncoding];
        }
        return true;
    }
    std::array<id<MTLComputePipelineState>, 11> functions;
};

MetalPlasmaEffect::MetalPlasmaEffect(int i) : PlasmaEffect(i) {
    data = new MetalPlasmaEffectData();
}
MetalPlasmaEffect::~MetalPlasmaEffect() {
    if (data) {
        delete data;
    }
}


void MetalPlasmaEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData * rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);

    const int ColorScheme = GetPlasmaColorScheme(SettingsMap["CHOICE_Plasma_Color"]);    
    if (rbcd == nullptr || !data->canRenderStyle(ColorScheme) || ((buffer.BufferWi * buffer.BufferHt) < 1024)) {
        PlasmaEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float oset = buffer.GetEffectTimeIntervalPosition();
    float Style = SettingsMap.GetInt("SLIDER_Plasma_Style", 1);
    float Line_Density = SettingsMap.GetInt("SLIDER_Plasma_Line_Density", 1);
    float PlasmaSpeed = GetValueCurveInt("Plasma_Speed", 10, SettingsMap, oset, PLASMA_SPEED_MIN, PLASMA_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    const float state = (buffer.curPeriod - buffer.curEffStartPer); // frames 0 to N
    const float Speed_plasma = (101.0f - PlasmaSpeed) * 3.0f; // we want a large number to divide by
    const float time = (state + 1.0) / Speed_plasma;
    const float sin_time_5 = sin(time / 5.0f);
    const float cos_time_3 = cos(time / 3.0f);
    const float sin_time_2 = sin(time / 2.0f);
    
    PlasmaData rdata;
    rdata.width = buffer.BufferWi;
    rdata.height = buffer.BufferHt;
    rdata.state = state;
    rdata.Style = Style;
    rdata.Line_Density = Line_Density;
    rdata.sin_time_5 = sin_time_5;
    rdata.cos_time_3 = cos_time_3;
    rdata.sin_time_2 = sin_time_2;
    rdata.time = time;

    rdata.numColors = buffer.palette.Size();
    for (int x = 0; x < rdata.numColors; x++) {
        rdata.colors[x] = buffer.palette.GetColor(x).asChar4();
    }

    if (data->Render(ColorScheme, rdata, buffer)) {
        return;
    }
    PlasmaEffect::Render(effect, SettingsMap, buffer);
}
