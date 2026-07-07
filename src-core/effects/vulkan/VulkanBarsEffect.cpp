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

#include <cmath>

#include "VulkanEffects.h"
#include "VulkanComputeUtilities.h"
#include "VulkanEffectDataTypes.h"

#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"

static xlvk::uchar4 toUchar4(const xlColor& c) {
    return { c.red, c.green, c.blue, c.alpha };
}

// Encode the Bars kernel into the render buffer's command stream.  Returns
// false (→ CPU fallback) if the GPU resources aren't available.
static bool renderBarsGPU(VulkanBarsData& bdata, RenderBuffer& buffer) {
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
    VkCommandBuffer cb = rbcd->getCommandBuffer("-Bars");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    uint32_t pixelCount = bdata.width * bdata.height;
    return rbcd->encodeEffectDispatch(cb, u.barsEffectFunction, "BarsEffect",
                                      &bdata, sizeof(bdata), { px.buffer }, pixelCount, 0);
}

VulkanBarsEffect::VulkanBarsEffect(int i) : BarsEffect(i) {
}
VulkanBarsEffect::~VulkanBarsEffect() {
}

void VulkanBarsEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        BarsEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float offset = buffer.GetEffectTimeIntervalPosition();
    int paletteRepeat = GetValueCurveInt("Bars_BarCount", sBarCountDefault, SettingsMap, offset, sBarCountMin, sBarCountMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    double cycles = GetValueCurveDouble("Bars_Cycles", sCyclesDefault, SettingsMap, offset, sCyclesMin, sCyclesMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sCyclesDivisor);
    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    double center = GetValueCurveDouble("Bars_Center", sCenterDefault, SettingsMap, position, sCenterMin, sCenterMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    const std::string& dirStr = SettingsMap["CHOICE_Bars_Direction"];
    int direction = 0;
    if      (dirStr == "up")             direction = 0;
    else if (dirStr == "down")           direction = 1;
    else if (dirStr == "expand")         direction = 2;
    else if (dirStr == "compress")       direction = 3;
    else if (dirStr == "Left")           direction = 4;
    else if (dirStr == "Right")          direction = 5;
    else if (dirStr == "H-expand")       direction = 6;
    else if (dirStr == "H-compress")     direction = 7;
    else if (dirStr == "Alternate Up")   direction = 8;
    else if (dirStr == "Alternate Down") direction = 9;
    else if (dirStr == "Alternate Left") direction = 10;
    else if (dirStr == "Alternate Right")direction = 11;
    else {
        // Custom Horz / Custom Vert — always spatial, fall back to CPU
        BarsEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    bool highlight               = SettingsMap.GetBool("CHECKBOX_Bars_Highlight", sHighlightDefault);
    bool useFirstColorForHighlight = highlight && SettingsMap.GetBool("CHECKBOX_Bars_UseFirstColorForHighlight", sUseFirstColorForHighlightDefault);
    bool show3D                  = SettingsMap.GetBool("CHECKBOX_Bars_3D", s3DDefault);
    bool gradient                = SettingsMap.GetBool("CHECKBOX_Bars_Gradient", sGradientDefault);

    size_t colorcnt = buffer.GetColorCount();
    if (colorcnt == 0) colorcnt = 1;

    if (highlight && useFirstColorForHighlight) {
        if (colorcnt == 1) {
            useFirstColorForHighlight = false;
        } else {
            colorcnt -= 1;
        }
    }

    // Fall back for spatial palette entries
    for (size_t i = 0; i < colorcnt; i++) {
        if (buffer.palette.IsSpatial(i)) {
            BarsEffect::Render(effect, SettingsMap, buffer);
            return;
        }
    }

    if ((int)colorcnt > MAX_VULKAN_BARS_COLORS) {
        BarsEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    int barCount = paletteRepeat * (int)colorcnt;
    if (barCount < 1) barCount = 1;

    VulkanBarsData bdata = {};
    bdata.width     = buffer.BufferWi;
    bdata.height    = buffer.BufferHt;
    bdata.colorCount= (int)colorcnt;
    bdata.highlight = highlight ? 1 : 0;
    bdata.show3D    = show3D    ? 1 : 0;
    bdata.gradient  = gradient  ? 1 : 0;
    bdata.allowAlpha= buffer.allowAlpha ? 1 : 0;
    bdata.useFirstColorForHighlight = useFirstColorForHighlight ? 1 : 0;

    if (useFirstColorForHighlight) {
        xlColor hc;
        buffer.palette.GetColor(0, hc);
        bdata.highlightColor = toUchar4(hc);
    } else {
        bdata.highlightColor = toUchar4(xlWHITE);
    }

    int colorOffset = useFirstColorForHighlight ? 1 : 0;
    for (int i = 0; i < (int)colorcnt; i++) {
        xlColor c;
        buffer.palette.GetColor(i + colorOffset, c);
        bdata.colorsAsRGBA[i] = toUchar4(c);
        HSVValue hsv = c.asHSV();
        bdata.colorsAsHSV[i] = { (float)hsv.hue, (float)hsv.saturation, (float)hsv.value };
    }

    if (direction < 4 || direction == 8 || direction == 9) {
        int barHt = (int)std::ceil((float)buffer.BufferHt / (float)barCount);
        if (barHt < 1) barHt = 1;
        int blockHt = (int)colorcnt * barHt;
        if (blockHt < 1) blockHt = 1;

        int f_offset = (int)(position * blockHt);
        if (direction == 8 || direction == 9) {
            f_offset = (int)(floor(position * barCount) * barHt);
        }
        int mappedDir = (direction > 4) ? direction - 8 : direction;

        bdata.direction  = mappedDir;
        bdata.barSize    = barHt;
        bdata.blockSize  = blockHt;
        bdata.f_offset   = f_offset;
        bdata.newCenter  = buffer.BufferHt * (100 + (int)center) / 200;
    } else {
        int barWi = (int)std::ceil((float)buffer.BufferWi / (float)barCount);
        if (barWi < 1) barWi = 1;
        int blockWi = (int)colorcnt * barWi;
        if (blockWi < 1) blockWi = 1;

        int f_offset = (int)(position * blockWi);
        if (direction > 9) {
            f_offset = (int)(floor(position * barCount) * barWi);
        }
        int mappedDir = (direction > 9) ? direction - 6 : direction;

        bdata.direction  = mappedDir;
        bdata.barSize    = barWi;
        bdata.blockSize  = blockWi;
        bdata.f_offset   = f_offset;
        bdata.newCenter  = buffer.BufferWi * (100 + (int)center) / 200;
    }

    if (renderBarsGPU(bdata, buffer)) {
        return;
    }
    BarsEffect::Render(effect, SettingsMap, buffer);
}

#endif
