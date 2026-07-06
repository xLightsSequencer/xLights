/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#pragma once

// Vulkan per-effect GPU wrappers (Phase D).  Each subclasses the CPU effect
// and overrides Render to dispatch a compute kernel, falling back to the base
// implementation for buffers below the size threshold or unsupported options.
// Mirrors MetalEffects.hpp.  Compiled only under HAVE_VULKAN.
#ifdef HAVE_VULKAN

#include "../BarsEffect.h"
#include "../ColorWashEffect.h"
#include "../ShockwaveEffect.h"
#include "../FanEffect.h"
#include "../SpiralsEffect.h"
#include "../GalaxyEffect.h"
#include "../CirclesEffect.h"
#include "../PlasmaEffect.h"
#include "../ButterflyEffect.h"
#include "../PinwheelEffect.h"
#include "../KaleidoscopeEffect.h"
#include "../WarpEffect.h"
#include "../TreeEffect.h"
#include "../ShimmerEffect.h"
#include "../CandleEffect.h"

class VulkanBarsEffect : public BarsEffect {
public:
    VulkanBarsEffect(int i);
    virtual ~VulkanBarsEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

class VulkanColorWashEffect : public ColorWashEffect {
public:
    VulkanColorWashEffect(int i);
    virtual ~VulkanColorWashEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

class VulkanShockwaveEffect : public ShockwaveEffect {
public:
    VulkanShockwaveEffect(int i);
    virtual ~VulkanShockwaveEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

class VulkanFanEffect : public FanEffect {
public:
    VulkanFanEffect(int i);
    virtual ~VulkanFanEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

class VulkanSpiralsEffect : public SpiralsEffect {
public:
    VulkanSpiralsEffect(int i);
    virtual ~VulkanSpiralsEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

class VulkanGalaxyEffect : public GalaxyEffect {
public:
    VulkanGalaxyEffect(int i);
    virtual ~VulkanGalaxyEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

class VulkanCirclesEffect : public CirclesEffect {
public:
    VulkanCirclesEffect(int i);
    virtual ~VulkanCirclesEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

class VulkanPlasmaEffect : public PlasmaEffect {
public:
    VulkanPlasmaEffect(int i);
    virtual ~VulkanPlasmaEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

class VulkanButterflyEffect : public ButterflyEffect {
public:
    VulkanButterflyEffect(int i);
    virtual ~VulkanButterflyEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

class VulkanPinwheelEffect : public PinwheelEffect {
public:
    VulkanPinwheelEffect(int i);
    virtual ~VulkanPinwheelEffect();

    // Pinwheel's GPU path hooks RenderNewArms (the base Render already parsed
    // settings into PinwheelData), not Render.
    virtual void RenderNewArms(RenderBuffer& buffer, PinwheelEffect::PinwheelData& data) override;
};

class VulkanKaleidoscopeEffect : public KaleidoscopeEffect {
public:
    VulkanKaleidoscopeEffect(int i);
    virtual ~VulkanKaleidoscopeEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

class VulkanWarpEffect : public WarpEffect {
public:
    VulkanWarpEffect(int i);
    virtual ~VulkanWarpEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

class VulkanTreeEffect : public TreeEffect {
public:
    VulkanTreeEffect(int i);
    virtual ~VulkanTreeEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

class VulkanShimmerEffect : public ShimmerEffect {
public:
    VulkanShimmerEffect(int i);
    virtual ~VulkanShimmerEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

class VulkanCandleEffect : public CandleEffect {
public:
    VulkanCandleEffect(int i);
    virtual ~VulkanCandleEffect();

    virtual void Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};

#endif
