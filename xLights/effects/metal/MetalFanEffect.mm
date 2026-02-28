#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../RenderBuffer.h"
#include "../FanEffect.h"

class MetalFanEffectData {
public:
    MetalFanEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("FanEffect");
    }
    ~MetalFanEffectData() {
        if (fn) { [fn release]; }
    }

    bool canRender() { return fn != nil; }

    bool Render(MetalFanData &fdata, RenderBuffer &buffer) {
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

            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            if (computeEncoder == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            [computeEncoder setLabel:@"FanEffect"];
            [computeEncoder setComputePipelineState:fn];

            NSInteger dataSize = sizeof(fdata);
            [computeEncoder setBytes:&fdata length:dataSize atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            NSInteger pixelCount = fdata.width * fdata.height;
            NSInteger threads = std::min(pixelCount, maxThreads);
            MTLSize gridSize = MTLSizeMake(pixelCount, 1, 1);
            MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);

            [computeEncoder dispatchThreads:gridSize
                      threadsPerThreadgroup:threadsPerThreadgroup];
            [computeEncoder endEncoding];
        }
        return true;
    }

private:
    id<MTLComputePipelineState> fn = nil;
};


MetalFanEffect::MetalFanEffect(int i) : FanEffect(i) {
    data = new MetalFanEffectData();
}
MetalFanEffect::~MetalFanEffect() {
    if (data) { delete data; }
}

void MetalFanEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || (buffer.BufferWi * buffer.BufferHt) < 2048) {
        FanEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    // Fall back to CPU if any palette entry uses spatial colors
    int num_colors = buffer.palette.Size();
    if (num_colors == 0) num_colors = 1;
    if (num_colors > MAX_METAL_FAN_COLORS) {
        FanEffect::Render(effect, SettingsMap, buffer);
        return;
    }
    for (int i = 0; i < num_colors; i++) {
        if (buffer.palette.IsSpatial(i)) {
            FanEffect::Render(effect, SettingsMap, buffer);
            return;
        }
    }

    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    int center_x    = GetValueCurveInt("Fan_CenterX",     50,  SettingsMap, eff_pos, FAN_CENTREX_MIN,     FAN_CENTREX_MAX,     buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int center_y    = GetValueCurveInt("Fan_CenterY",     50,  SettingsMap, eff_pos, FAN_CENTREY_MIN,     FAN_CENTREY_MAX,     buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_radius= GetValueCurveInt("Fan_Start_Radius", 1,  SettingsMap, eff_pos, FAN_STARTRADIUS_MIN, FAN_STARTRADIUS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_radius  = GetValueCurveInt("Fan_End_Radius",  10,  SettingsMap, eff_pos, FAN_ENDRADIUS_MIN,   FAN_ENDRADIUS_MAX,   buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_angle = GetValueCurveInt("Fan_Start_Angle",  0,  SettingsMap, eff_pos, FAN_STARTANGLE_MIN,  FAN_STARTANGLE_MAX,  buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int revolutions = GetValueCurveInt("Fan_Revolutions", 720, SettingsMap, eff_pos, FAN_REVOLUTIONS_MIN, FAN_REVOLUTIONS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 360);
    int num_blades  = GetValueCurveInt("Fan_Num_Blades",   3,  SettingsMap, eff_pos, FAN_BLADES_MIN,      FAN_BLADES_MAX,      buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int blade_width = GetValueCurveInt("Fan_Blade_Width",  50, SettingsMap, eff_pos, FAN_BLADEWIDTH_MIN,  FAN_BLADEWIDTH_MAX,  buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int blade_angle = GetValueCurveInt("Fan_Blade_Angle",  90, SettingsMap, eff_pos, FAN_BLADEANGLE_MIN,  FAN_BLADEANGLE_MAX,  buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int num_elements= GetValueCurveInt("Fan_Num_Elements",  1, SettingsMap, eff_pos, FAN_NUMELEMENTS_MIN, FAN_NUMELEMENTS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int element_width=GetValueCurveInt("Fan_Element_Width",100, SettingsMap, eff_pos, FAN_ELEMENTWIDTH_MIN,FAN_ELEMENTWIDTH_MAX,buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int duration    = GetValueCurveInt("Fan_Duration",     80, SettingsMap, eff_pos, FAN_DURATION_MIN,    FAN_DURATION_MAX,    buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int acceleration= GetValueCurveInt("Fan_Accel",         0, SettingsMap, eff_pos, FAN_ACCEL_MIN,       FAN_ACCEL_MAX,       buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool reverse_dir  = SettingsMap.GetBool("CHECKBOX_Fan_Reverse");
    bool blend_edges  = SettingsMap.GetBool("CHECKBOX_Fan_Blend_Edges");
    bool scale        = SettingsMap.GetBool("CHECKBOX_Fan_Scale", true);

    double eff_pos_adj = buffer.calcAccel(eff_pos, acceleration);
    double revs = (double)revolutions;

    double effect_duration = duration / 100.0;
    double radius_rampup   = (1.0 - effect_duration) / 2.0;

    double radius1 = start_radius;
    double radius2 = end_radius;

    if (scale) {
        double bufferMax = std::max(buffer.BufferHt, buffer.BufferWi);
        radius1 = radius1 * (bufferMax / 200.0);
        radius2 = radius2 * (bufferMax / 200.0);
        start_radius = (int)(start_radius * (bufferMax / 200.0));
        end_radius   = (int)(end_radius   * (bufferMax / 200.0));
    }

    if (effect_duration < 1.0) {
        double radius_delta = std::abs(radius2 - radius1);
        if (eff_pos_adj < radius_rampup) {
            double pct = 1.0 - (eff_pos_adj / radius_rampup);
            if (radius2 > radius1) radius2 = radius2 - radius_delta * pct;
            else                   radius2 = radius2 + radius_delta * pct;
        } else if (eff_pos_adj > (1.0 - radius_rampup)) {
            double pct = (1.0 - eff_pos_adj) / radius_rampup;
            if (radius2 > radius1) radius1 = radius2 - radius_delta * pct;
            else                   radius1 = radius2 + radius_delta * pct;
        }
    }

    if (radius1 > radius2) std::swap(radius1, radius2);

    int max_radius = std::max(start_radius, end_radius);

    double blade_div_angle  = 360.0 / (double)num_blades;
    double blade_width_angle= blade_div_angle * (double)blade_width / 100.0;
    double color_angle      = blade_width_angle / (double)num_colors;
    double angle_offset     = eff_pos_adj * revs + (double)start_angle;
    double element_angle    = color_angle / (double)num_elements;
    double element_size     = element_angle * (double)element_width / 100.0;

    MetalFanData fdata;
    fdata.width  = buffer.BufferWi;
    fdata.height = buffer.BufferHt;
    fdata.xc_adj = (center_x - 50) * buffer.BufferWi / 100;
    fdata.yc_adj = (center_y - 50) * buffer.BufferHt / 100;
    fdata.radius1         = (float)radius1;
    fdata.radius2         = (float)radius2;
    fdata.max_radius      = (float)max_radius;
    fdata.blade_div_angle = (float)blade_div_angle;
    fdata.blade_width_angle=(float)blade_width_angle;
    fdata.color_angle     = (float)color_angle;
    fdata.element_angle   = (float)element_angle;
    fdata.element_size    = (float)element_size;
    fdata.angle_offset    = (float)(eff_pos_adj * revs);
    fdata.start_angle     = (float)start_angle;
    fdata.blade_angle     = (float)blade_angle;
    fdata.reverse_dir     = reverse_dir ? 1 : 0;
    fdata.blend_edges     = blend_edges ? 1 : 0;
    fdata.allowAlpha      = buffer.allowAlpha ? 1 : 0;
    fdata.num_colors      = num_colors;

    for (int i = 0; i < num_colors; i++) {
        xlColor c;
        buffer.palette.GetColor(i, c);
        fdata.colorsAsRGBA[i] = c.asChar4();
        HSVValue hsv = c;
        fdata.colorsAsHSV[i] = { (float)hsv.hue, (float)hsv.saturation, (float)hsv.value };
    }

    if (data->Render(fdata, buffer)) {
        return;
    }
    FanEffect::Render(effect, SettingsMap, buffer);
}
