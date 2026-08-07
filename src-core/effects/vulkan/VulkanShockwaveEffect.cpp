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

#include "../../render/Effect.h"
#include "../../render/RenderBuffer.h"
#include "../../render/SequenceElements.h"
#include "UtilClasses.h"

static xlvk::uchar4 toUchar4(const xlColor& c) {
    return { c.red, c.green, c.blue, c.alpha };
}

static bool renderShockwaveGPU(VulkanShockwaveData& rdata, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }
    VkCommandBuffer cb = rbcd->getCommandBuffer("-Shockwave");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    uint32_t pixelCount = rdata.width * rdata.height;
    return rbcd->encodeEffectDispatch(cb, u.shockwaveEffectFunction, "ShockwaveEffect",
                                      &rdata, sizeof(rdata), { px.buffer }, pixelCount, 0);
}

VulkanShockwaveEffect::VulkanShockwaveEffect(int i) : ShockwaveEffect(i) {
}
VulkanShockwaveEffect::~VulkanShockwaveEffect() {
}

void VulkanShockwaveEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        ShockwaveEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    VulkanShockwaveData rdata = {};
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
            // nothing to draw yet (buffer stays cleared)
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
    // Mirror ShockwaveEffect::Render's DOUBLE param math (not float), narrowing
    // to float only when filling the kernel struct - same reasoning as the
    // Metal wrapper: float here shifts color_index and the ring edges vs the
    // ISPC/CPU reference. See MetalShockwaveEffect.mm.
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
    if (scale) {
        double bufferMax = std::max(buffer.BufferHt, buffer.BufferWi);
        radius1 = radius1 * (bufferMax / 200.0);
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

    rdata.color = toUchar4(color);
    HSVValue hsv = color.asHSV();
    rdata.colorHSV = { (float)hsv.hue, (float)hsv.saturation, (float)hsv.value };
    rdata.blend = blend_edges ? 1 : 0;
    rdata.allowAlpha = buffer.allowAlpha ? 1 : 0;

    if (renderShockwaveGPU(rdata, buffer)) {
        return;
    }
    ShockwaveEffect::Render(effect, SettingsMap, buffer);
}

#endif
