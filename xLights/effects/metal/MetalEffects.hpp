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



