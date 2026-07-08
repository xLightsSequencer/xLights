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

#include "VulkanEffects.h"
#include "VulkanComputeUtilities.h"
#include "VulkanEffectDataTypes.h"

#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"

static xlvk::uchar4 toUchar4(const xlColor& c) {
    return { c.red, c.green, c.blue, c.alpha };
}

// Encode the Fan kernel into the render buffer's command stream.  Returns
// false (-> CPU fallback) if the GPU resources aren't available.
static bool renderFanGPU(VulkanFanData& fdata, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    // Buffers before command buffer (a grow can reset the command pool).
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }
    VkCommandBuffer cb = rbcd->getCommandBuffer("-Fan");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    uint32_t pixelCount = fdata.width * fdata.height;
    return rbcd->encodeEffectDispatch(cb, u.fanEffectFunction, "FanEffect",
                                      &fdata, sizeof(fdata), { px.buffer }, pixelCount, 0);
}

VulkanFanEffect::VulkanFanEffect(int i) : FanEffect(i) {
}
VulkanFanEffect::~VulkanFanEffect() {
}

void VulkanFanEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        FanEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    // Fall back to CPU if any palette entry uses spatial colors
    int num_colors = buffer.palette.Size();
    if (num_colors == 0) num_colors = 1;
    if (num_colors > MAX_VULKAN_FAN_COLORS) {
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
    int center_x     = GetValueCurveInt("Fan_CenterX",      sCenterXDefault,     SettingsMap, eff_pos, sCenterXMin,     sCenterXMax,     buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int center_y     = GetValueCurveInt("Fan_CenterY",      sCenterYDefault,     SettingsMap, eff_pos, sCenterYMin,     sCenterYMax,     buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_radius = GetValueCurveInt("Fan_Start_Radius", sStartRadiusDefault, SettingsMap, eff_pos, sStartRadiusMin, sStartRadiusMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_radius   = GetValueCurveInt("Fan_End_Radius",   sEndRadiusDefault,   SettingsMap, eff_pos, sEndRadiusMin,   sEndRadiusMax,   buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_angle  = GetValueCurveInt("Fan_Start_Angle",  sStartAngleDefault,  SettingsMap, eff_pos, sStartAngleMin,  sStartAngleMax,  buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int revolutions  = GetValueCurveInt("Fan_Revolutions",  sRevolutionsDefault, SettingsMap, eff_pos, sRevolutionsMin, sRevolutionsMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sRevolutionsDivisor);
    int num_blades   = GetValueCurveInt("Fan_Num_Blades",   sNumBladesDefault,   SettingsMap, eff_pos, sNumBladesMin,   sNumBladesMax,   buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int blade_width  = GetValueCurveInt("Fan_Blade_Width",  sBladeWidthDefault,  SettingsMap, eff_pos, sBladeWidthMin,  sBladeWidthMax,  buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int blade_angle  = GetValueCurveInt("Fan_Blade_Angle",  sBladeAngleDefault,  SettingsMap, eff_pos, sBladeAngleMin,  sBladeAngleMax,  buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int num_elements = GetValueCurveInt("Fan_Num_Elements", sNumElementsDefault, SettingsMap, eff_pos, sNumElementsMin, sNumElementsMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int element_width= GetValueCurveInt("Fan_Element_Width",sElementWidthDefault,SettingsMap, eff_pos, sElementWidthMin,sElementWidthMax,buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int duration     = GetValueCurveInt("Fan_Duration",     sDurationDefault,    SettingsMap, eff_pos, sDurationMin,    sDurationMax,    buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int acceleration = GetValueCurveInt("Fan_Accel",        sAccelDefault,       SettingsMap, eff_pos, sAccelMin,       sAccelMax,       buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool reverse_dir  = SettingsMap.GetBool("CHECKBOX_Fan_Reverse",     sReverseDefault);
    bool blend_edges  = SettingsMap.GetBool("CHECKBOX_Fan_Blend_Edges", sBlendEdgesDefault);
    bool scale        = SettingsMap.GetBool("CHECKBOX_Fan_Scale",       sScaleDefault);

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
    double element_angle    = color_angle / (double)num_elements;
    double element_size     = element_angle * (double)element_width / 100.0;

    VulkanFanData fdata = {};
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
        fdata.colorsAsRGBA[i] = toUchar4(c);
        HSVValue hsv = c;
        fdata.colorsAsHSV[i] = { (float)hsv.hue, (float)hsv.saturation, (float)hsv.value };
    }

    if (renderFanGPU(fdata, buffer)) {
        return;
    }
    FanEffect::Render(effect, SettingsMap, buffer);
}

#endif
