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
#include "UtilClasses.h"

// The kernel updates the per-node CandleState array in place, so the state
// buffer is uploaded, the dispatch is committed and waited on immediately
// (synchronous, unlike the other Vulkan effects which defer commit to the
// blend pass), and the result is copied back into the CPU-side cache before
// Render() returns -- the cache stays the single source of truth and the
// CPU/Vulkan paths remain interchangeable mid-sequence. Mirrors
// MetalCandleEffectData::Render in MetalCandleEffect.mm.
//
// CandleState (CandleEffect.h) is 5 packed uint8_t fields (static_assert
// sizeof==5) with no natural 4-byte alignment. GLSL storage buffers have no
// portable 8-bit element type (no 8-bit-storage device feature is enabled
// in VulkanComputeUtilities::createDeviceAndQueue), so each field is widened
// to a full uint32 in VulkanCandleState purely for the GPU round trip (see
// VulkanEffectDataTypes.h) -- every record then owns 5 whole words
// exclusively, so the shader can read/write them with plain (non-atomic)
// loads/stores with no cross-thread partial-word races. This makes the
// upload/readback a small per-state pack/unpack loop rather than a literal
// memcpy of the CandleState vector.
static bool renderCandleGPU(VulkanCandleData& cd, std::vector<CandleState>& states, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }

    size_t numStates = states.size();
    size_t statesSize = numStates * sizeof(VulkanCandleState);
    VulkanBuffer stateBuf;
    if (!u.createSharedBuffer(stateBuf, statesSize, "CandleState")) {
        return false;
    }
    VulkanCandleState* gpuStates = (VulkanCandleState*)stateBuf.mapped;
    for (size_t i = 0; i < numStates; i++) {
        gpuStates[i].flameprimer = states[i].flameprimer;
        gpuStates[i].flamer = states[i].flamer;
        gpuStates[i].wind = states[i].wind;
        gpuStates[i].flameprimeg = states[i].flameprimeg;
        gpuStates[i].flameg = states[i].flameg;
    }

    VkCommandBuffer cb = rbcd->getCommandBuffer("-Candle");
    if (cb == VK_NULL_HANDLE) {
        u.destroyBuffer(stateBuf);
        return false;
    }

    bool ok = rbcd->encodeEffectDispatch(cb, u.candleEffectFunction, "CandleEffect",
                                         &cd, sizeof(cd), { px.buffer, stateBuf.buffer },
                                         cd.width * cd.height, 0);
    if (ok) {
        rbcd->commit();
        rbcd->waitForCompletion();
        for (size_t i = 0; i < numStates; i++) {
            states[i].flameprimer = (uint8_t)gpuStates[i].flameprimer;
            states[i].flamer = (uint8_t)gpuStates[i].flamer;
            states[i].wind = (uint8_t)gpuStates[i].wind;
            states[i].flameprimeg = (uint8_t)gpuStates[i].flameprimeg;
            states[i].flameg = (uint8_t)gpuStates[i].flameg;
        }
    }
    u.destroyBuffer(stateBuf);
    return ok;
}

VulkanCandleEffect::VulkanCandleEffect(int i) : CandleEffect(i) {
}
VulkanCandleEffect::~VulkanCandleEffect() {
}

void VulkanCandleEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        CandleEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    // Vulkan implements the perNode flame simulation only. The single-flame
    // mode, DMX buffers, and out-of-range params fall back to the CPU path
    // (which itself does ISPC/scalar) -- mirrors MetalCandleEffect::Render.
    bool perNode = SettingsMap.GetBool("CHECKBOX_PerNode", sPerNodeDefault);
    if (!perNode || buffer.IsDmxBuffer()) {
        CandleEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float oset = buffer.GetEffectTimeIntervalPosition();
    int flameAgility = GetValueCurveInt("Candle_FlameAgility", sFlameAgilityDefault, SettingsMap, oset, sFlameAgilityMin, sFlameAgilityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windCalmness = GetValueCurveInt("Candle_WindCalmness", sWindCalmnessDefault, SettingsMap, oset, sWindCalmnessMin, sWindCalmnessMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windVariability = GetValueCurveInt("Candle_WindVariability", sWindVariabilityDefault, SettingsMap, oset, sWindVariabilityMin, sWindVariabilityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windBaseline = GetValueCurveInt("Candle_WindBaseline", sWindBaselineDefault, SettingsMap, oset, sWindBaselineMin, sWindBaselineMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool usePalette = SettingsMap.GetBool("CHECKBOX_UsePalette", sUsePaletteDefault);

    size_t npix = (size_t)buffer.BufferWi * buffer.BufferHt;
    if (npix > buffer.GetPixelCount()
        || windVariability < 0 || windVariability > 255
        || windBaseline < 0 || windBaseline > 255
        || windCalmness < 0 || windCalmness > 31
        || flameAgility < 0 || flameAgility > 254) {
        CandleEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    const auto& pal = effect->GetPalette();
    xlColor c1, c2;
    if (usePalette) {
        if (pal.empty()) {
            c1 = xlWHITE;
            c2 = xlBLACK;
        } else {
            c1 = pal[0];
            c2 = (pal.size() > 1 ? pal[1] : xlBLACK);
        }
    }

    int maxW = 0;
    std::vector<CandleState>& states = *getPerNodeStates(buffer, SettingsMap, maxW);
    if (states.empty()) {
        CandleEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    VulkanCandleData cd = {};
    cd.width = buffer.BufferWi;
    cd.height = buffer.BufferHt;
    cd.maxWid = maxW;
    cd.numStates = (uint32_t)states.size();
    uint64_t frameSeed = buffer.hashRandomFrameSeed();
    cd.frameSeedLo = (uint32_t)(frameSeed & 0xFFFFFFFFu);
    cd.frameSeedHi = (uint32_t)(frameSeed >> 32);
    cd.windVariability = windVariability;
    cd.flameAgility = flameAgility;
    cd.windCalmness = windCalmness;
    cd.windBaseline = windBaseline;
    cd.usePalette = usePalette ? 1 : 0;
    // Alpha is forced to 255, matching MetalCandleEffect::Render exactly
    // (not the actual palette color's alpha channel).
    cd.c1 = { c1.red, c1.green, c1.blue, 255 };
    cd.c2 = { c2.red, c2.green, c2.blue, 255 };

    if (renderCandleGPU(cd, states, buffer)) {
        return;
    }
    CandleEffect::Render(effect, SettingsMap, buffer);
}

#endif
