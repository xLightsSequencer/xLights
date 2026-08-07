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

#include "../../render/Effect.h"
#include "../../render/RenderBuffer.h"
#include "../../render/SequenceElements.h"
#include "UtilClasses.h"

static xlvk::uchar4 toUchar4(const xlColor& c) {
    return { c.red, c.green, c.blue, c.alpha };
}

static bool renderPinwheelGPU(VulkanPinwheelData& rdata, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }
    VkCommandBuffer cb = rbcd->getCommandBuffer("-Pinwheel");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    uint32_t pixelCount = rdata.width * rdata.height;
    return rbcd->encodeEffectDispatch(cb, u.pinwheelEffectFunction, "PinwheelEffect",
                                      &rdata, sizeof(rdata), { px.buffer }, pixelCount, 0);
}

VulkanPinwheelEffect::VulkanPinwheelEffect(int i) : PinwheelEffect(i) {
}
VulkanPinwheelEffect::~VulkanPinwheelEffect() {
}

void VulkanPinwheelEffect::RenderNewArms(RenderBuffer& buffer, PinwheelEffect::PinwheelData& data) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold
        || data.hasSpacial || (data.colorsAsColor.size() >= MAX_VULKAN_PINWHEEL_ARMS)) {
        PinwheelEffect::RenderNewArms(buffer, data);
        return;
    }

    VulkanPinwheelData rdata = {};
    rdata.width = buffer.BufferWi;
    rdata.height = buffer.BufferHt;
    rdata.pinwheel_arms = data.pinwheel_arms;
    rdata.xc_adj = data.xc_adj;
    rdata.yc_adj = data.yc_adj;
    rdata.degrees_per_arm = data.degrees_per_arm;
    rdata.pinwheel_twist = data.pinwheel_twist;
    rdata.max_radius = data.max_radius;
    rdata.poffset = data.poffset;
    rdata.pw3dType = data.pw3dType;
    rdata.pinwheel_rotation = data.pinwheel_rotation ? 1 : 0;
    rdata.tmax = data.tmax;
    rdata.pos = data.pos;
    rdata.allowAlpha = buffer.allowAlpha ? 1 : 0;

    rdata.numColors = (int32_t)data.colorsAsColor.size();
    for (int x = 0; x < rdata.numColors; x++) {
        rdata.colorsAsColor[x] = toUchar4(data.colorsAsColor[x]);
        HSVValue hsv = data.colorsAsHSV[x];
        rdata.colorsAsHSV[x] = { (float)hsv.hue, (float)hsv.saturation, (float)hsv.value };
    }

    if (renderPinwheelGPU(rdata, buffer)) {
        return;
    }
    PinwheelEffect::RenderNewArms(buffer, data);
}

#endif
