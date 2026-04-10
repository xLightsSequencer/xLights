#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"

class ColorWashEffect : public RenderableEffect
{
public:
    ColorWashEffect(int id);
    virtual ~ColorWashEffect();

    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2, xlVertexColorAccumulator& bg, xlColor* colorMask, bool ramps) override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    // Cached from ColorWash.json at startup. Cycles uses divisor=10, so the
    // stored VC form is Min=1|Max=200 with the effect's divisor dividing the
    // raw ticks to produce 0.1..20 cycles.
    static double sCyclesDefault;
    static double sCyclesMin;
    static double sCyclesMax;
    static int sCyclesDivisor;
    static bool sVFadeDefault;
    static bool sHFadeDefault;
    static bool sReverseFadesDefault;
    static bool sShimmerDefault;
    static bool sCircularPaletteDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
