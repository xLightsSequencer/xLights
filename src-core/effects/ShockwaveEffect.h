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

class ShockwaveEffect : public RenderableEffect
{
public:
    ShockwaveEffect(int id);
    virtual ~ShockwaveEffect();
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2,
                                     xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps) override;
    virtual bool SupportsRadialColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }

    static int sCenterXDefault;
    static int sCenterXMin;
    static int sCenterXMax;
    static int sCenterYDefault;
    static int sCenterYMin;
    static int sCenterYMax;
    static int sStartRadiusDefault;
    static int sStartRadiusMin;
    static int sStartRadiusMax;
    static int sEndRadiusDefault;
    static int sEndRadiusMin;
    static int sEndRadiusMax;
    static int sStartWidthDefault;
    static int sStartWidthMin;
    static int sStartWidthMax;
    static int sEndWidthDefault;
    static int sEndWidthMin;
    static int sEndWidthMax;
    static int sAccelDefault;
    static int sCyclesDefault;
    static bool sScaleDefault;
    static bool sBlendEdgesDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
