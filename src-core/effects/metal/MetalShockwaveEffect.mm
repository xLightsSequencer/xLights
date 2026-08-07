
#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../render/Effect.h"
#include "../../render/RenderBuffer.h"
#include "../../render/SequenceElements.h"

#include "UtilClasses.h"

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
    if (rbcd == nullptr || ((buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold)) {
        ShockwaveEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    
    MetalShockwaveData rdata;
    rdata.width = buffer.BufferWi;
    rdata.height = buffer.BufferHt;
        
    std::string timingtrack = SettingsMap.Get("CHOICE_Shockwave_TimingTrack", "");
    if (buffer.needToInit) {
        buffer.needToInit = false;
        if (!timingtrack.empty()) {
            effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->AddRenderDependency(timingtrack, buffer.cur_model);
        }
    }
    double eff_pos;
    if (timingtrack.empty()) {
        int cycles = SettingsMap.GetInt("SLIDER_Shockwave_Cycles", sCyclesDefault);
        if (cycles < 1) cycles = 1;
        eff_pos = buffer.GetEffectTimeIntervalPosition(cycles);
    } else {
        eff_pos = getEffectPosition(buffer, SettingsMap, timingtrack);
        if (eff_pos < 0) {
            //nothing to draw yet
            return;
        }
    }
    
    int center_x = GetValueCurveInt("Shockwave_CenterX", sCenterXDefault, SettingsMap, eff_pos, sCenterXMin, sCenterXMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int center_y = GetValueCurveInt("Shockwave_CenterY", sCenterYDefault, SettingsMap, eff_pos, sCenterYMin, sCenterYMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_radius = GetValueCurveInt("Shockwave_Start_Radius", sStartRadiusDefault, SettingsMap, eff_pos, sStartRadiusMin, sStartRadiusMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_radius = GetValueCurveInt("Shockwave_End_Radius", sEndRadiusDefault, SettingsMap, eff_pos, sEndRadiusMin, sEndRadiusMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_width = GetValueCurveInt("Shockwave_Start_Width", sStartWidthDefault, SettingsMap, eff_pos, sStartWidthMin, sStartWidthMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_width = GetValueCurveInt("Shockwave_End_Width", sEndWidthDefault, SettingsMap, eff_pos, sEndWidthMin, sEndWidthMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int acceleration = SettingsMap.GetInt("SLIDER_Shockwave_Accel", sAccelDefault);
    bool blend_edges = SettingsMap.GetBool("CHECKBOX_Shockwave_Blend_Edges", sBlendEdgesDefault);
    bool scale = SettingsMap.GetBool("CHECKBOX_Shockwave_Scale", sScaleDefault);

    int num_colors = buffer.palette.Size();
    if (num_colors == 0) {
        num_colors = 1;
    }
    // These param computations MUST mirror ShockwaveEffect::Render's DOUBLE
    // math exactly (not float): the CPU/ISPC path the sub-threshold buffers
    // and Windows/Linux use computes color_index / ring radii in double then
    // narrows to float only when filling the kernel struct. Doing it in float
    // here shifted color_index (wrong color pair) and the ring edges by a ULP
    // -> byte-parity break vs ISPC. Compute in double, narrow at assignment.
    double eff_pos_adj = buffer.calcAccel(eff_pos, acceleration);

    xlColor color;
    double blend_pct = 1.0;
    if (num_colors > 1) {
        blend_pct = 1.0 / (num_colors - 1);
    }
    double color_pct1 = eff_pos_adj / blend_pct;
    int color_index = (int)color_pct1;
    blend_pct = color_pct1 - (double)color_index;
    buffer.Get2ColorBlend(std::min(color_index, num_colors - 1), std::min(color_index + 1, num_colors - 1), std::min(blend_pct, 1.0), color);
    if (buffer.palette.IsSpatial(color_index)) {
        ShockwaveEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    rdata.xc_adj = center_x * buffer.BufferWi / 100;
    rdata.yc_adj = center_y * buffer.BufferHt / 100;

    double radius1 = start_radius;
    double radius2 = end_radius;
    if (scale) { // convert to percentage of buffer, i.e 100 is 100% of buffer size
        double bufferMax = std::max(buffer.BufferHt, buffer.BufferWi);
        radius1 = radius1 * (bufferMax / 200.0); // 200 bc radius is half of the width
        radius2 = radius2 * (bufferMax / 200.0);
        start_width = start_width * (bufferMax / 100.0);
        end_width = end_width * (bufferMax / 100.0);
    }
    double radius_center = radius1 + (radius2 - radius1) * eff_pos_adj;
    double half_width = (start_width + (end_width - start_width) * eff_pos_adj) / 2.0;
    if (half_width < 0.25) {
        half_width = 0.25;
    }
    radius1 = radius_center - half_width;
    radius2 = radius_center + half_width;
    radius1 = std::max(0.0, radius1);

    rdata.radius_center = (float)radius_center;
    rdata.half_width = (float)half_width;
    rdata.radius1 = (float)radius1;
    rdata.radius2 = (float)radius2;

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
