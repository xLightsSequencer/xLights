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

class GalaxyEffect : public RenderableEffect
{
public:
    GalaxyEffect(int id);
    virtual ~GalaxyEffect();

    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2,
                                     xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps) override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }

    // Cached from Galaxy.json by OnMetadataLoaded().
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
    static int sStartAngleDefault;
    static int sStartAngleMin;
    static int sStartAngleMax;
    // Galaxy_Revolutions is stored pre-divisor (raw int) in both the VC and
    // Render path. JSON lists the default as the post-divisor float (e.g., 4.0),
    // so OnMetadataLoaded multiplies by the divisor to get the pre-divisor tick
    // count for sRevolutionsDefault.
    static int sRevolutionsDefault;
    static int sRevolutionsMin;
    static int sRevolutionsMax;
    static int sRevolutionsDivisor;
    static int sStartWidthDefault;
    static int sStartWidthMin;
    static int sStartWidthMax;
    static int sEndWidthDefault;
    static int sEndWidthMin;
    static int sEndWidthMax;
    static int sDurationDefault;
    static int sDurationMin;
    static int sDurationMax;
    static int sAccelDefault;
    static int sAccelMin;
    static int sAccelMax;
    static bool sReverseDefault;
    static bool sBlendEdgesDefault;
    static bool sInwardDefault;
    static bool sScaleDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
