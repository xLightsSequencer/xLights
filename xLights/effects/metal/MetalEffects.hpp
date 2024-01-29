#pragma once

#include "../ButterflyEffect.h"
#include "../PlasmaEffect.h"
#include "../WarpEffect.h"
#include "../PinwheelEffect.h"

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
