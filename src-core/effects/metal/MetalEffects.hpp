#pragma once

#include "../ButterflyEffect.h"
#include "../PlasmaEffect.h"
#include "../WarpEffect.h"
#include "../PinwheelEffect.h"
#include "../ShockwaveEffect.h"
#include "../KaleidoscopeEffect.h"
#include "../FanEffect.h"
#include "../SpiralsEffect.h"
#include "../ColorWashEffect.h"
#include "../BarsEffect.h"
#include "../CirclesEffect.h"
#include "../GalaxyEffect.h"
#include "../TreeEffect.h"
#include "../TwinkleEffect.h"
#include "../ShimmerEffect.h"
#include "../CandleEffect.h"
#include "../LifeEffect.h"
#include "../WaveEffect.h"
#include "../GarlandsEffect.h"
#include "../ShaderEffect.h"
#include "../FillEffect.h"
#include "../MeteorsEffect.h"

class MetalButterflyEffectData;

class MetalButterflyEffect : public ButterflyEffect {
public:
    MetalButterflyEffect(int i);
    virtual ~MetalButterflyEffect();


    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;


private:
    MetalButterflyEffectData *data;
};


class MetalWarpEffectData;

class MetalWarpEffect : public WarpEffect {
public:
    MetalWarpEffect(int i);
    virtual ~MetalWarpEffect();


    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;


private:
    MetalWarpEffectData *data;
};


class MetalPlasmaEffectData;

class MetalPlasmaEffect : public PlasmaEffect {
public:
    MetalPlasmaEffect(int i);
    virtual ~MetalPlasmaEffect();


    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;


private:
    MetalPlasmaEffectData *data;
};

class MetalPinwheelEffectData;

class MetalPinwheelEffect : public PinwheelEffect {
public:
    MetalPinwheelEffect(int i);
    virtual ~MetalPinwheelEffect();

    virtual void RenderNewArms(RenderBuffer& buffer, PinwheelEffect::PinwheelData &data) override;
private:
    MetalPinwheelEffectData *data;
};


class MetalShockwaveEffectData;

class MetalShockwaveEffect : public ShockwaveEffect {
public:
    MetalShockwaveEffect(int i);
    virtual ~MetalShockwaveEffect();


    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;


private:
    MetalShockwaveEffectData *data;
};


class MetalFanEffectData;

class MetalFanEffect : public FanEffect {
public:
    MetalFanEffect(int i);
    virtual ~MetalFanEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalFanEffectData *data;
};


class MetalKaleidoscopeEffectData;

class MetalKaleidoscopeEffect : public KaleidoscopeEffect {
public:
    MetalKaleidoscopeEffect(int i);
    virtual ~MetalKaleidoscopeEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalKaleidoscopeEffectData *data;
};

class MetalSpiralsEffectData;

class MetalSpiralsEffect : public SpiralsEffect {
public:
    MetalSpiralsEffect(int i);
    virtual ~MetalSpiralsEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalSpiralsEffectData *data;
};

class MetalGalaxyEffectData;

class MetalGalaxyEffect : public GalaxyEffect {
public:
    MetalGalaxyEffect(int i);
    virtual ~MetalGalaxyEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalGalaxyEffectData *data;
};

class MetalTreeEffectData;

class MetalTreeEffect : public TreeEffect {
public:
    MetalTreeEffect(int i);
    virtual ~MetalTreeEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalTreeEffectData *data;
};

class MetalTwinkleEffectData;

class MetalTwinkleEffect : public TwinkleEffect {
public:
    MetalTwinkleEffect(int i);
    virtual ~MetalTwinkleEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalTwinkleEffectData *data;
};

class MetalShimmerEffectData;

class MetalShimmerEffect : public ShimmerEffect {
public:
    MetalShimmerEffect(int i);
    virtual ~MetalShimmerEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalShimmerEffectData *data;
};
class MetalFillEffectData;

class MetalFillEffect : public FillEffect {
public:
    MetalFillEffect(int i);
    virtual ~MetalFillEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalFillEffectData *data;
};

class MetalMeteorsEffectData;

class MetalMeteorsEffect : public MeteorsEffect {
public:
    MetalMeteorsEffect(int i);
    virtual ~MetalMeteorsEffect();

protected:
    // Overrides only the per-pixel gather; the base handles all the CPU bookkeeping
    // and the radial styles. Falls back to the base (ISPC) when Metal isn't viable.
    virtual void GatherMeteors(RenderBuffer& buffer, const MeteorsGatherParams& params, const std::vector<MeteorSnapshot>& parts) override;

private:
    MetalMeteorsEffectData *data;
};

class MetalCandleEffectData;

class MetalCandleEffect : public CandleEffect {
public:
    MetalCandleEffect(int i);
    virtual ~MetalCandleEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalCandleEffectData *data;
};

class MetalLifeEffectData;

class MetalLifeEffect : public LifeEffect {
public:
    MetalLifeEffect(int i);
    virtual ~MetalLifeEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalLifeEffectData *data;
};

class MetalWaveEffectData;

class MetalWaveEffect : public WaveEffect {
public:
    MetalWaveEffect(int i);
    virtual ~MetalWaveEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalWaveEffectData *data;
};

class MetalGarlandsEffectData;

class MetalGarlandsEffect : public GarlandsEffect {
public:
    MetalGarlandsEffect(int i);
    virtual ~MetalGarlandsEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalGarlandsEffectData *data;
};

class MetalColorWashEffectData;

class MetalColorWashEffect : public ColorWashEffect {
public:
    MetalColorWashEffect(int i);
    virtual ~MetalColorWashEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalColorWashEffectData *data;
};


class MetalBarsEffectData;

class MetalBarsEffect : public BarsEffect {
public:
    MetalBarsEffect(int i);
    virtual ~MetalBarsEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalBarsEffectData *data;
};


class MetalCirclesEffectData;
class MetalCirclesEffect : public CirclesEffect {
public:
    MetalCirclesEffect(int i);
    virtual ~MetalCirclesEffect();

    virtual void Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) override;

private:
    MetalCirclesEffectData *data;
};


class MetalShaderEffect : public ShaderEffect {
public:
    MetalShaderEffect(int i);
    virtual ~MetalShaderEffect();

    virtual void preparePixelTextures(RenderBuffer& buffer, bool shadersInit, unsigned fbId) override;
    virtual void copyPixelDataToTexture(RenderBuffer& buffer, unsigned rbTex) override;
    virtual void copyPixelDataFromTexture(RenderBuffer& buffer) override;

    // Native Metal render path (opt-in via XL_NATIVE_SHADER). Translates the ISF
    // GLSL to MSL and renders through a Metal pipeline instead of OpenGL; falls
    // back to the base OpenGL ShaderEffect::Render on anything unsupported.
    virtual void Render(Effect* eff, const SettingsMap& SettingsMap, RenderBuffer& buffer) override;
};
