
#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../RenderBuffer.h"

#include <array>

class MetalShockwaveEffectData {
public:
    MetalShockwaveEffectData() {
        for (auto &f : functions) {
            f = nil;
        }
        functions[0] = MetalComputeUtilities::INSTANCE.FindComputeFunction("ShockwaveEffectStyle0");
        //functions[1] = MetalComputeUtilities::INSTANCE.FindComputeFunction("ShockwaveEffectStyle1");
        //functions[2] = MetalComputeUtilities::INSTANCE.FindComputeFunction("ShockwaveEffectStyle2");
        //functions[3] = MetalComputeUtilities::INSTANCE.FindComputeFunction("ShockwaveEffectStyle3");
        //functions[4] = MetalComputeUtilities::INSTANCE.FindComputeFunction("ShockwaveEffectStyle4");
        //functions[5] = MetalComputeUtilities::INSTANCE.FindComputeFunction("ShockwaveEffectStyle5");
    }
    ~MetalShockwaveEffectData() {
        for (auto &f : functions) {
            if (f != nil) {
                [f release];
            }
        }
    }
    bool canRenderStyle(int style) {
        return style < functions.size() && functions[style] != nil;
    }

    bool Render(MetalShockwaveData &data, RenderBuffer &buffer) {
        @autoreleasepool {
            MetalRenderBufferComputeData * rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);

            id<MTLCommandBuffer> commandBuffer = rbcd->getCommandBuffer();
            if (commandBuffer == nil) {
                return false;
            }
            
            id<MTLBuffer> bufferResult = rbcd->getPixelBuffer();
            if (bufferResult == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            if (computeEncoder == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            [computeEncoder setLabel:@"ShockwaveEffect"];
            [computeEncoder setComputePipelineState:functions[0]];

            NSInteger dataSize = sizeof(data);
            [computeEncoder setBytes:&data length:dataSize atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];

            NSInteger maxThreads = functions[0].maxTotalThreadsPerThreadgroup;
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

MetalShockwaveEffect::MetalShockwaveEffect(int i) : ShockwaveEffect(i) {
    data = new MetalShockwaveEffectData();
}
MetalShockwaveEffect::~MetalShockwaveEffect() {
    if (data) {
        delete data;
    }
}


void MetalShockwaveEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData * rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || ((buffer.BufferWi * buffer.BufferHt) < 1024)) {
        ShockwaveEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    
    MetalShockwaveData rdata;
    rdata.width = buffer.BufferWi;
    rdata.height = buffer.BufferHt;
    
    int cycles = SettingsMap.GetInt("SLIDER_Shockwave_Cycles", 1);
    double eff_pos = buffer.GetEffectTimeIntervalPosition(cycles);
    int center_x = GetValueCurveInt("Shockwave_CenterX", 50, SettingsMap, eff_pos, SHOCKWAVE_X_MIN, SHOCKWAVE_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int center_y = GetValueCurveInt("Shockwave_CenterY", 50, SettingsMap, eff_pos, SHOCKWAVE_Y_MIN, SHOCKWAVE_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_radius = GetValueCurveInt("Shockwave_Start_Radius", 0, SettingsMap, eff_pos, SHOCKWAVE_STARTRADIUS_MIN, SHOCKWAVE_STARTRADIUS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_radius = GetValueCurveInt("Shockwave_End_Radius", 0, SettingsMap, eff_pos, SHOCKWAVE_ENDRADIUS_MIN, SHOCKWAVE_ENDRADIUS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_width = GetValueCurveInt("Shockwave_Start_Width", 0, SettingsMap, eff_pos, SHOCKWAVE_STARTWIDTH_MIN, SHOCKWAVE_STARTWIDTH_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_width = GetValueCurveInt("Shockwave_End_Width", 0, SettingsMap, eff_pos, SHOCKWAVE_ENDWIDTH_MIN, SHOCKWAVE_ENDWIDTH_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int acceleration = SettingsMap.GetInt("SLIDER_Shockwave_Accel", 0);
    bool blend_edges = SettingsMap.GetBool("CHECKBOX_Shockwave_Blend_Edges");
    bool scale = SettingsMap.GetBool("CHECKBOX_Shockwave_Scale", false);

    int num_colors = buffer.palette.Size();
    if (num_colors == 0) {
        num_colors = 1;
    }
    float eff_pos_adj = buffer.calcAccel(eff_pos, acceleration);

    xlColor color;
    float blend_pct = 1.0;
    if (num_colors > 1) {
        blend_pct = 1.0 / (num_colors - 1);
    }
    float color_pct1 = eff_pos_adj / blend_pct;
    int color_index = (int)color_pct1;
    blend_pct = color_pct1 - (double)color_index;
    buffer.Get2ColorBlend(std::min(color_index, num_colors - 1), std::min(color_index + 1, num_colors - 1), std::min(blend_pct, 1.0f), color);
    if (buffer.palette.IsSpatial(color_index)) {
        ShockwaveEffect::Render(effect, SettingsMap, buffer);
        return;
    }
    
    rdata.xc_adj = center_x * buffer.BufferWi / 100;
    rdata.yc_adj = center_y * buffer.BufferHt / 100;

    rdata.radius1 = start_radius;
    rdata.radius2 = end_radius;
    if (scale) { // convert to percentage of buffer, i.e 100 is 100% of buffer size
        double bufferMax = std::max(buffer.BufferHt, buffer.BufferWi);
        rdata.radius1 = rdata.radius1 * (bufferMax / 200.0); // 200 bc radius is half of the width
        rdata.radius2 = rdata.radius2 * (bufferMax / 200.0);
        start_width = start_width * (bufferMax / 100.0);
        end_width = end_width * (bufferMax / 100.0);
    }
    rdata.radius_center = rdata.radius1 + (rdata.radius2 - rdata.radius1) * eff_pos_adj;
    rdata.half_width = (start_width + (end_width - start_width) * eff_pos_adj) / 2.0;
    if (rdata.half_width < 0.25) {
        rdata.half_width = 0.25;
    }
    rdata.radius1 = rdata.radius_center - rdata.half_width;
    rdata.radius2 = rdata.radius_center + rdata.half_width;
    rdata.radius1 = std::max(0.0f, rdata.radius1);
    
    rdata.color = color.asChar4();
    auto hsv = color.asHSV();
    rdata.colorHSV = {(float)hsv.hue, (float)hsv.saturation, (float)hsv.value};
    rdata.blend = blend_edges;
    rdata.allowAlpha = buffer.allowAlpha;

    if (data->Render(rdata, buffer)) {
        return;
    }
    ShockwaveEffect::Render(effect, SettingsMap, buffer);
}
