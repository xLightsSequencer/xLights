#pragma once

#include "../ButterflyEffect.h"


class MetalButterflyEffectData;

class MetalButterflyEffect : public ButterflyEffect {
public:
    MetalButterflyEffect(int i);
    virtual ~MetalButterflyEffect();


    virtual void Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) override;


private:
    MetalButterflyEffectData *data;
};
