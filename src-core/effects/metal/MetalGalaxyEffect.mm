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
#include "../GalaxyEffect.h"

class MetalGalaxyEffectData {
public:
    MetalGalaxyEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("GalaxyEffect");
    }

    bool canRender() { return fn != nil; }

    bool Render(MetalGalaxyData &gdata, RenderBuffer &buffer) {
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
            [computeEncoder setLabel:@"GalaxyEffect"];
            [computeEncoder setComputePipelineState:fn];

            NSInteger dataSize = sizeof(gdata);
            [computeEncoder setBytes:&gdata length:dataSize atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            NSInteger pixelCount = gdata.width * gdata.height;
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


MetalGalaxyEffect::MetalGalaxyEffect(int i) : GalaxyEffect(i) {
    data = new MetalGalaxyEffectData();
}
MetalGalaxyEffect::~MetalGalaxyEffect() {
    if (data) { delete data; }
}

void MetalGalaxyEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || (buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold) {
        GalaxyEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    // Metal implements the "New Render Method" gather only. Old style, DMX buffers, and
    // palettes over the color limit fall back to the CPU path (which itself does ISPC/scalar).
    const std::string &renderStyle = SettingsMap.Get("CHOICE_Galaxy_RenderStyle", sRenderStyleDefault);
    int num_colors = buffer.palette.Size();
    if (renderStyle == "Old Render Method" || buffer.IsDmxBuffer()
        || num_colors <= 0 || num_colors > MAX_METAL_GALAXY_COLORS) {
        GalaxyEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    int center_x = GetValueCurveInt("Galaxy_CenterX", sCenterXDefault, SettingsMap, eff_pos, sCenterXMin, sCenterXMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int center_y = GetValueCurveInt("Galaxy_CenterY", sCenterYDefault, SettingsMap, eff_pos, sCenterYMin, sCenterYMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_radius = GetValueCurveInt("Galaxy_Start_Radius", sStartRadiusDefault, SettingsMap, eff_pos, sStartRadiusMin, sStartRadiusMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_radius = GetValueCurveInt("Galaxy_End_Radius", sEndRadiusDefault, SettingsMap, eff_pos, sEndRadiusMin, sEndRadiusMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_angle = GetValueCurveInt("Galaxy_Start_Angle", sStartAngleDefault, SettingsMap, eff_pos, sStartAngleMin, sStartAngleMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int revolutions = GetValueCurveInt("Galaxy_Revolutions", sRevolutionsDefault, SettingsMap, eff_pos, sRevolutionsMin, sRevolutionsMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sRevolutionsDivisor);
    int start_width = GetValueCurveInt("Galaxy_Start_Width", sStartWidthDefault, SettingsMap, eff_pos, sStartWidthMin, sStartWidthMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_width = GetValueCurveInt("Galaxy_End_Width", sEndWidthDefault, SettingsMap, eff_pos, sEndWidthMin, sEndWidthMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int duration = GetValueCurveInt("Galaxy_Duration", sDurationDefault, SettingsMap, eff_pos, sDurationMin, sDurationMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int acceleration = GetValueCurveInt("Galaxy_Accel", sAccelDefault, SettingsMap, eff_pos, sAccelMin, sAccelMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool reverse_dir = SettingsMap.GetBool("CHECKBOX_Galaxy_Reverse", sReverseDefault);
    bool blend_edges = SettingsMap.GetBool("CHECKBOX_Galaxy_Blend_Edges", sBlendEdgesDefault);
    bool inward = SettingsMap.GetBool("CHECKBOX_Galaxy_Inward", sInwardDefault);
    bool scale = SettingsMap.GetBool("CHECKBOX_Galaxy_Scale", sScaleDefault);

    if (revolutions == 0) {
        GalaxyEffect::Render(effect, SettingsMap, buffer); // returns immediately, draws nothing
        return;
    }

    double eff_pos_adj = buffer.calcAccel(eff_pos, acceleration);
    double revs = (double)revolutions;
    double pos_x = buffer.BufferWi * center_x / 100.0;
    double pos_y = buffer.BufferHt * center_y / 100.0;
    double head_duration = duration / 100.0;
    double tail_length = revs * (1.0 - head_duration);
    double color_length = tail_length / num_colors;
    if (color_length < 1.0) color_length = 1.0;
    double tail_end_of_tail = ((revs + tail_length) * eff_pos_adj) - tail_length;
    double head_end_of_tail = tail_end_of_tail + tail_length;
    double radius1 = start_radius, radius2 = end_radius, width1 = start_width, width2 = end_width;
    if (scale) {
        double bufferMax = std::max(buffer.BufferHt, buffer.BufferWi);
        radius1 = radius1 * (bufferMax / 200.0);
        radius2 = radius2 * (bufferMax / 200.0);
        width1 = width1 * (bufferMax / 100.0);
        width2 = width2 * (bufferMax / 100.0);
    }

    MetalGalaxyData gd;
    gd.width = buffer.BufferWi;
    gd.height = buffer.BufferHt;
    gd.pos_x = (float)pos_x;
    gd.pos_y = (float)pos_y;
    gd.radius1 = (float)radius1;
    gd.radius2 = (float)radius2;
    gd.width1 = (float)width1;
    gd.width2 = (float)width2;
    gd.revs = (float)revs;
    gd.start_angle = (float)start_angle;
    gd.reverse_dir = reverse_dir ? 1 : 0;
    gd.inward = inward ? 1 : 0;
    gd.blend_edges = blend_edges ? 1 : 0;
    gd.head_end_of_tail = (float)head_end_of_tail;
    gd.tail_end_of_tail = (float)tail_end_of_tail;
    gd.color_length = (float)color_length;
    gd.num_colors = num_colors;
    for (int i = 0; i < MAX_METAL_GALAXY_COLORS; i++) {
        if (i < num_colors) {
            xlColor c;
            buffer.palette.GetColor(i, c);
            gd.palR[i] = c.red; gd.palG[i] = c.green; gd.palB[i] = c.blue;
        } else {
            gd.palR[i] = gd.palG[i] = gd.palB[i] = 0;
        }
    }

    if (data->Render(gd, buffer)) {
        return;
    }
    GalaxyEffect::Render(effect, SettingsMap, buffer);
}
