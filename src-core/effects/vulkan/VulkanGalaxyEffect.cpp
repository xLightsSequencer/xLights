/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#ifdef HAVE_VULKAN

#include <algorithm>

#include "VulkanEffects.h"
#include "VulkanComputeUtilities.h"
#include "VulkanEffectDataTypes.h"

#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"

static bool renderGalaxyGPU(VulkanGalaxyData& gd, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }
    VkCommandBuffer cb = rbcd->getCommandBuffer("-Galaxy");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    uint32_t pixelCount = gd.width * gd.height;
    return rbcd->encodeEffectDispatch(cb, u.galaxyEffectFunction, "GalaxyEffect",
                                      &gd, sizeof(gd), { px.buffer }, pixelCount, 0);
}

VulkanGalaxyEffect::VulkanGalaxyEffect(int i) : GalaxyEffect(i) {
}
VulkanGalaxyEffect::~VulkanGalaxyEffect() {
}

void VulkanGalaxyEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        GalaxyEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    int num_colors = buffer.palette.Size();
    if (buffer.IsDmxBuffer()
        || num_colors <= 0 || num_colors > MAX_VULKAN_GALAXY_COLORS) {
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

    VulkanGalaxyData gd = {};
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
    for (int i = 0; i < MAX_VULKAN_GALAXY_COLORS; i++) {
        if (i < num_colors) {
            xlColor c;
            buffer.palette.GetColor(i, c);
            gd.palR[i] = c.red; gd.palG[i] = c.green; gd.palB[i] = c.blue;
        } else {
            gd.palR[i] = gd.palG[i] = gd.palB[i] = 0;
        }
    }

    if (renderGalaxyGPU(gd, buffer)) {
        return;
    }
    GalaxyEffect::Render(effect, SettingsMap, buffer);
}

#endif
